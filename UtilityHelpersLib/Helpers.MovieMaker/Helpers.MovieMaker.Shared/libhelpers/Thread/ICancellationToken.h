#pragma once

namespace thread {
	class ICancellationToken {
	public:
		ICancellationToken();
		virtual ~ICancellationToken();

		virtual bool IsCancelled() const = 0;
		virtual void CancelTask() = 0;
	};

	inline void CheckCancel(ICancellationToken* ct) {
		if (ct && ct->IsCancelled()) {
			ct->CancelTask();
		}
	}
}
