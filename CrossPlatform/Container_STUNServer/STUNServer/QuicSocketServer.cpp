#include "QuicSocketServer.h"
#include "MsQuicInitializer.h"
#include "System.h"
#include "Logger.h"

#define NOMINMAX

#ifdef __UNIX_OS__
#define OutputDebugStringA(text) (void)(0)
#endif

namespace {
#if defined(__WINDOWS_OS__)

	std::string ExePath() {
		char buffer[MAX_PATH] = { 0 };
		GetModuleFileNameA(NULL, buffer, MAX_PATH);
		auto pos = std::string{ buffer }.find_last_of("\\/");
		return std::string{ buffer }.substr(0, pos);
	}

#elif defined(__UNIX_OS__)

	std::string ExePath() {
		char buffer[1024] = { 0 };
		readlink("/proc/self/exe", buffer, 1024);
		std::string path = buffer;
		return path.substr(0, path.rfind("/"));
	}

	void Error(const std::string& error) {
		const std::string red("\033[0;31m");
		const std::string green("\033[1;32m");
		const std::string yellow("\033[1;33m");
		const std::string reset("\033[0m");

		std::cerr << std::endl << red << error << reset << std::endl;
	}

	std::string ExecuteShellCommand(std::string command) {
		char buffer[256];
		std::string result = "";
		const char* cmd = command.c_str();
		FILE* pipe = popen(cmd, "r");
		if (!pipe) throw std::runtime_error("popen() failed!");

		try {
			while (!feof(pipe))
				if (fgets(buffer, 128, pipe) != NULL)
					result += buffer;
		}
		catch (...) {
			pclose(pipe);
			throw;
		}
		pclose(pipe);
		return result;
	}

	int CountProcessesListenPort(std::string port) {
		auto output = ExecuteShellCommand("lsof -i:" + port);

		int pos = 0;
		int counter = 0;
		while ((pos = output.find(port, pos)) != std::string::npos) {
			pos += port.size();
			counter++;
		}
		return counter;
	}
#endif

	std::uint64_t GenerateId() {
		static std::uint64_t currentId = 0;
		return currentId++;
	}
}

QuicSocketServer::QuicSocketServer(uint16_t port) : listeningPort(port) {
#if defined(__UNIX_OS__)
	int countProcesses = CountProcessesListenPort(std::to_string(port));
	if (countProcesses > 0) {
		Error("Other processes(" + std::to_string(countProcesses) + ") are already using this port. Process was terminated.");
		exit(1);
	}
#endif // !WIN32 && !WIN64

	const auto& msQuic = MsQuicInitializer::GetInstance().GetQuicApiTable();

	auto settings = QUIC_SETTINGS{ 0 };

	const uint32_t Version = 0xff00001dU; // IETF draft 29
	settings.DesiredVersionsList = &Version;
	settings.DesiredVersionsListLength = 1;
	settings.IsSet.DesiredVersionsList = TRUE;

	settings.IdleTimeoutMs = 15000;
	settings.IsSet.IdleTimeoutMs = true;

	settings.PeerBidiStreamCount = std::numeric_limits<std::uint16_t>::max();
	//settings.PeerBidiStreamCount = 10;
	settings.IsSet.PeerBidiStreamCount = true;

	auto hr = msQuic.ConfigurationOpen(MsQuicInitializer::GetInstance().GetRegistration(), &alpn, 1, &settings, sizeof(settings), nullptr, &configuration);
	H::System::ThrowIfFailed(hr);

}

QuicSocketServer::~QuicSocketServer() {
	const auto& msQuic = MsQuicInitializer::GetInstance().GetQuicApiTable();

	//msQuic.StreamShutdown(stream, QUIC_STREAM_SHUTDOWN_FLAG_GRACEFUL, 0);
	//msQuic.StreamClose(stream);
}

void QuicSocketServer::StartListening() {

	const auto& msQuic = MsQuicInitializer::GetInstance().GetQuicApiTable();
	auto creds = QUIC_CREDENTIAL_CONFIG{};
	memset(&creds, 0, sizeof(creds));

	creds.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE;

	//TODO change path
	auto certFile = QUIC_CERTIFICATE_FILE{ 0 };
	auto exePath = ExePath();
	auto certFilePath = exePath + "/host.cert";
	auto privateKeyFilePath = exePath + "/host.key";
	certFile.CertificateFile = certFilePath.c_str();
	certFile.PrivateKeyFile = privateKeyFilePath.c_str();
	creds.CertificateFile = &certFile;

	//hr = msQuic.ConfigurationLoadCredential(*configuration.get(), &creds);
	auto hr = msQuic.ConfigurationLoadCredential(configuration, &creds);
	H::System::ThrowIfFailed(hr);

	hr = msQuic.ListenerOpen(MsQuicInitializer::GetInstance().GetRegistration(), ServerListenerCallback, this, &listener);
	H::System::ThrowIfFailed(hr);

	auto address = QUIC_ADDR{};
	QuicAddrSetFamily(&address, QUIC_ADDRESS_FAMILY_UNSPEC);
	QuicAddrSetPort(&address, listeningPort);


	hr = msQuic.ListenerStart(listener, &alpn, 1, &address);
	H::System::ThrowIfFailed(hr);
	//puncher.reset(nullptr);

}

QUIC_STATUS QuicSocketServer::ServerListenerCallback(HQUIC listener, void* context, QUIC_LISTENER_EVENT* event)
{
	auto status = QUIC_STATUS_NOT_SUPPORTED;
	const auto& msQuic = MsQuicInitializer::GetInstance().GetQuicApiTable();

	auto _this = static_cast<QuicSocketServer*>(context);
	UserAddress address;
	switch (event->Type) {
	case QUIC_LISTENER_EVENT_NEW_CONNECTION:
		address = _this->GetGlobalAddress(event->NEW_CONNECTION.Connection);
		WriteDebug("QuicSocketServer::ServerListenerCallback New connection ip: %s:%d", address.ip.c_str(), address.port);

		msQuic.SetCallbackHandler(event->NEW_CONNECTION.Connection, &QuicSocketServer::ServerConnectionCallback, context);
		status = msQuic.ConnectionSetConfiguration(event->NEW_CONNECTION.Connection, _this->configuration);

		break;
	default:
		break;
	}
	return status;
}

QUIC_STATUS QuicSocketServer::ServerConnectionCallback(HQUIC connection, void* context, QUIC_CONNECTION_EVENT* event)
{
	const auto& msQuic = MsQuicInitializer::GetInstance().GetQuicApiTable();
	auto _this = static_cast<QuicSocketServer*>(context);
	auto address = _this->GetGlobalAddress(connection);

	switch (event->Type) {
	case QUIC_CONNECTION_EVENT_CONNECTED:
		//
		// The handshake has completed for the connection.
		//
		WriteDebug("QuicSocketServer::ServerConnectionCallback Connected ip: %s:%d", address.ip.c_str(), address.port);
		WriteDebug("QuicSocketServer::ServerConnectionCallback Active connections = %d", ++_this->activeConnections);
		msQuic.ConnectionSendResumptionTicket(connection, QUIC_SEND_RESUMPTION_FLAG_NONE, 0, NULL);
		break;
	case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
		//
		// The connection has been shut down by the transport. Generally, this
		// is the expected way for the connection to shut down with this
		// protocol, since we let idle timeout kill the connection.
		//
		WriteDebug("QuicSocketServer::ServerConnectionCallback Shut down by transport, 0x%x, ip: %s:%d", 
			event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status, address.ip.c_str(), address.port);
		break;
	case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
		//
		// The connection was explicitly shut down by the peer.
		//
		WriteDebug("QuicSocketServer::ServerConnectionCallback Shut down by peer, 0x%llu, ip: %s:%d", 
			(unsigned long long)event->SHUTDOWN_INITIATED_BY_PEER.ErrorCode, address.ip.c_str(), address.port);
		break;
	case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
		//
		// The connection has completed the shutdown process and is ready to be
		// safely cleaned up.
		//
		//{
		//	auto lock = std::lock_guard{ _this->usersMutex };
		//	_this->users.DeleteUserByGlobalAddress(_this->GetGlobalAddress(connection));
		//}
		if (connection == nullptr) {
			WriteError("QuicSocketServer::ServerConnectionCallback connection is nullptr during shutdown event handling, ip: %s:%d",
				address.ip.c_str(), address.port);
			break;
		}
		msQuic.ConnectionClose(connection);
		WriteDebug("QuicSocketServer::ServerConnectionCallback connection was closed during shutdown handling, ip: %s:%d",
			address.ip.c_str(), address.port);
		WriteDebug("QuicSocketServer::ServerConnectionCallback Active connections = %d", --_this->activeConnections);
		//msQuic.StreamClose(_this->stream);
		//_this->cv.notify_all();
		break;
	case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED: {

		auto newActiveUser = ActiveUser{};
		newActiveUser.globalAddress = address;

		auto context = new StreamContext();
		context->id = GenerateId();
		context->server = _this;
		WriteDebug("QuicSocketServer::ServerConnectionCallback Started stream with new client address: %s:%d , id = %u", 
			newActiveUser.globalAddress.ip.c_str(), newActiveUser.globalAddress.port, context->id);
		{
			auto lock = std::lock_guard{ _this->usersMutex };
			_this->users.AddNewUser(context->id, newActiveUser);
		}

		msQuic.SetCallbackHandler(event->PEER_STREAM_STARTED.Stream, (void*)ServerStreamCallback, context);

		break;
	}
	case QUIC_CONNECTION_EVENT_RESUMED:
		//
		// The connection succeeded in doing a TLS resumption of a previous
		// connection's session.
		//
		break;
	default:
		break;
	}
	return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS QuicSocketServer::ServerStreamCallback(HQUIC stream, void* context, QUIC_STREAM_EVENT* event) {
	const auto& msQuic = MsQuicInitializer::GetInstance().GetQuicApiTable();
	auto streamContext = static_cast<StreamContext*>(context);
	auto _this = streamContext->server;

	auto userMessageID = streamContext->id;
	switch (event->Type) {
	case QUIC_STREAM_EVENT_SEND_COMPLETE: {
		//
		// A previous StreamSend call has completed, and the context is being
		// returned back to the app.
		//
		delete event->SEND_COMPLETE.ClientContext;
		OutputDebugStringA("QUIC_STREAM_EVENT_SEND_COMPLETE\n");

		break;
	}
	case QUIC_STREAM_EVENT_RECEIVE: {
		auto buffers = event->RECEIVE.Buffers;

		OutputDebugStringA("QUIC_STREAM_EVENT_RECEIVE\n");
		auto totalSize = event->RECEIVE.TotalBufferLength;

		auto messageBuffer = std::string{};

		for (int i = 0; i < event->RECEIVE.BufferCount; ++i) {
			messageBuffer.append(reinterpret_cast<char*>(event->RECEIVE.Buffers[i].Buffer), event->RECEIVE.Buffers[i].Length);
		}

		auto processedBytes = uint32_t{ 0 };
		if (_this->usersMessages[userMessageID].messageSize == 0) {
			_this->usersMessages[userMessageID].messageSize = *reinterpret_cast<const uint32_t*>(messageBuffer.data());

			messageBuffer.erase(messageBuffer.begin(), messageBuffer.begin() + sizeof(uint32_t));
			processedBytes += sizeof(uint32_t);
		}

		if (_this->usersMessages[userMessageID].chunkedMessage.size() + totalSize - processedBytes < _this->usersMessages[userMessageID].messageSize) {
			//assert(totalSize - processedBytes >= 0);
			if (totalSize - processedBytes < 0)
				WriteError("QuicSocketServer::ServerStreamCallback::QUIC_STREAM_EVENT_RECEIVE totalSize - processedBytes < 0 client id: %u", userMessageID);
			auto message = std::string{ messageBuffer.data(), totalSize - processedBytes };
			_this->usersMessages[userMessageID].chunkedMessage.append(message);
			MsQuicInitializer::GetInstance().GetQuicApiTable().StreamReceiveComplete(stream, totalSize);
			return QUIC_STATUS_SUCCESS;
		}
		else {
			WriteDebug("QuicSocketServer::ServerStreamCallback::QUIC_STREAM_EVENT_RECEIVE start message processing for client id: %u", userMessageID);
			//assert(_this->chunkedMessage.size() + totalSize >= processedBytes + _this->messageSize);
			//assert(_this->messageSize - _this->chunkedMessage.size() > 0);
			if (_this->usersMessages[userMessageID].chunkedMessage.size() + totalSize < processedBytes + _this->usersMessages[userMessageID].messageSize)
				WriteError("QuicSocketServer::ServerStreamCallback::QUIC_STREAM_EVENT_RECEIVE processing not whole message");

			if (_this->usersMessages[userMessageID].messageSize - _this->usersMessages[userMessageID].chunkedMessage.size() <= 0)
				WriteError("QuicSocketServer::ServerStreamCallback::QUIC_STREAM_EVENT_RECEIVE message already full");

			auto message = std::string{ messageBuffer.data(), 
				_this->usersMessages[userMessageID].messageSize - _this->usersMessages[userMessageID].chunkedMessage.size() };

			processedBytes += _this->usersMessages[userMessageID].messageSize - _this->usersMessages[userMessageID].chunkedMessage.size();
			_this->usersMessages[userMessageID].chunkedMessage.append(message);

			assert(processedBytes <= totalSize);
			MsQuicInitializer::GetInstance().GetQuicApiTable().StreamReceiveComplete(stream, processedBytes);
			if (processedBytes != totalSize) {
				WriteDebug("QuicSocketServer::ServerStreamCallback::QUIC_STREAM_EVENT_RECEIVE processedBytes != totalSize");
				int stop11 = 5464;
				OutputDebugStringA("Processed bytes not match total: ");
				OutputDebugStringA(std::to_string(totalSize - processedBytes).c_str());
				OutputDebugStringA("\n");
				MsQuicInitializer::GetInstance().GetQuicApiTable().StreamReceiveSetEnabled(stream, true);
			}

			//assert(_this->messageSize == _this->chunkedMessage.size());
			if(_this->usersMessages[userMessageID].messageSize != _this->usersMessages[userMessageID].chunkedMessage.size())
				WriteError("QuicSocketServer::ServerStreamCallback::QUIC_STREAM_EVENT_RECEIVE actual message size not match with expected");

			_this->usersMessages[userMessageID].chunkedMessage.resize(_this->usersMessages[userMessageID].messageSize);
			auto userAuthData = _this->serializer.DeserializeUserAuthMessage(_this->usersMessages[userMessageID].chunkedMessage);
			_this->usersMessages.erase(userMessageID);

			if (!_this->jwtReader.IsValidToken(userAuthData.jwt)) {
				WriteError("QuicSocketServer::ServerStreamCallback::QUIC_STREAM_EVENT_RECEIVE Token validation failed, token value: %s", userAuthData.jwt.c_str());
				msQuic.StreamShutdown(stream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT_SEND, 0);
				return QUIC_STATUS_SUCCESS;
			}
			// If we response with local address then users are in same network. 
			userAuthData.localAddress.sameNetworkWithPartner = true;

			_this->users.SetUserData(userMessageID, userAuthData.requesterUserServerId, userAuthData.localAddress);
			_this->users.FetchUser(userMessageID, userAuthData.requestedUserServerId, [stream, _this, userMessageID, userAuthData](const UserAddress& targetUser) {
				 WriteDebug("UserDataExchanger FetchUser fetching user %u for %u done from callback", userAuthData.requestedUserServerId, userMessageID);
				 WriteDebug("UserDataExchanger Send message to user with id: $u", userMessageID);
				_this->SendMessageToClient(_this->serializer.SerializeIpPort(targetUser), stream);
				});

			WriteDebug("QuicSocketServer::ServerStreamCallback::QUIC_STREAM_EVENT_RECEIVE message processing finished for user with id: %u", userMessageID);

		}

		return QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN: {
		//
		// The peer gracefully shut down its send direction of the stream.
		//
		{
			auto lock = std::lock_guard{ _this->usersMutex };
			if (stream != nullptr)
				_this->users.DeleteUser(userMessageID);
			else
				WriteError("QuicSocketServer::ServerStreamCallback::QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN stream is nullptr, user id: %u", userMessageID);
		}


		WriteDebug("QuicSocketServer::ServerStreamCallback Peer send shut down, user id: %u", userMessageID);
		break;
	}
	case QUIC_STREAM_EVENT_PEER_SEND_ABORTED: {
		if (stream == nullptr) {
			WriteError("QuicSocketServer::ServerStreamCallback::QUIC_STREAM_EVENT_PEER_SEND_ABORTED stream is nullptr, user id: %u", userMessageID);
			break;
		}

		{
			auto lock = std::lock_guard{ _this->usersMutex };
			_this->users.DeleteUser(userMessageID);
		}

		WriteDebug("QuicSocketServer::ServerStreamCallback Peer send aborted, user id: %u", userMessageID);
		msQuic.StreamShutdown(stream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
		break;
	}
	case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
		WriteDebug("QuicSocketServer::ServerStreamCallback shutdown complete, user id: %u", userMessageID);
		//
		// Both directions of the stream have been shut down and MsQuic is done
		// with the stream. It can now be safely cleaned up.
		//

		if (stream == nullptr) {
			WriteError("QuicSocketServer::ServerStreamCallback::QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE stream is nullptr, user id: %u", userMessageID);
			break;
		}

		{
			auto lock = std::lock_guard{ _this->usersMutex };
			_this->users.DeleteUser(userMessageID);
			_this->usersMessages.erase(userMessageID);
			if (streamContext != nullptr)
				delete streamContext;
		}
		WriteDebug("QuicSocketServer::ServerStreamCallback All done for user id: %u", userMessageID);
		msQuic.StreamClose(stream);
		break;
	case QUIC_STREAM_EVENT_PEER_RECEIVE_ABORTED:
		WriteDebug("QuicSocketServer::ServerStreamCallback peer receive aborted, user id: %u", userMessageID);
		break;
	default:
		break;
	}
	return QUIC_STATUS_SUCCESS;
}

void QuicSocketServer::SendMessageToClient(std::string message, HQUIC stream) {
	WriteDebug("QuicSocketServer::SendMessageToClient start sending message");
	const auto& msQuic = MsQuicInitializer::GetInstance().GetQuicApiTable();

	auto messageLenBuffer = new QUIC_BUFFER{};
	messageLenBuffer->Length = sizeof(uint32_t);
	messageLenBuffer->Buffer = new uint8_t[sizeof(uint32_t)];
	uint32_t messageSize = message.size();
	std::memcpy(messageLenBuffer->Buffer, &messageSize, sizeof(uint32_t));

	assert(*reinterpret_cast<const uint32_t*>(messageLenBuffer->Buffer) == message.size());

	auto hr = msQuic.StreamSend(stream, messageLenBuffer, 1, QUIC_SEND_FLAG_NONE, messageLenBuffer);
	if (QUIC_FAILED(hr)) {
		WriteError("QuicSocketServer::SendMessageToClient Failed to send message, messageLenBuffer size = %i,hr = %x", messageSize, hr);
	}
	H::System::ThrowIfFailed(hr);

	auto buffer = new QUIC_BUFFER{};
	buffer->Length = message.size();
	buffer->Buffer = new uint8_t[message.size()];
	std::memcpy(buffer->Buffer, message.data(), message.size());

	hr = msQuic.StreamSend(stream, buffer, 1, QUIC_SEND_FLAG_NONE, buffer);
	if (QUIC_FAILED(hr)) {
		WriteError("QuicSocketServer::SendMessageToClient Failed to send message, buffer size = %i,hr = %x", message.size(), hr);
	}
	H::System::ThrowIfFailed(hr);

	WriteDebug("Exiting SendMessageToClient, message sent successfully");
}

void QuicSocketServer::HandleUserMessage() {
}

UserAddress QuicSocketServer::GetGlobalAddress(HQUIC connection) {
	const auto& msQuic = MsQuicInitializer::GetInstance().GetQuicApiTable();

	auto remoteAddres = QUIC_ADDR{ 0 };
	auto addresStructSize = static_cast<uint32_t>(sizeof(remoteAddres));
	msQuic.GetParam(connection, QUIC_PARAM_LEVEL_CONNECTION, QUIC_PARAM_CONN_REMOTE_ADDRESS, &addresStructSize, &remoteAddres);

	auto address = UserAddress{};
	auto addr = std::string{};
	addr.resize(INET_ADDRSTRLEN);

	inet_ntop(AF_INET, &remoteAddres.Ipv4.sin_addr, addr.data(), INET_ADDRSTRLEN);
	addr.erase(std::remove(addr.begin(), addr.end(), '\0'), addr.end());
	//auto adr = inet_ntop(AF_INET, &(sa.sin_addr), addr.data(), INET_ADDRSTRLEN);
	address.ip = addr;
	address.port = ntohs(remoteAddres.Ipv4.sin_port);

	return address;
}
