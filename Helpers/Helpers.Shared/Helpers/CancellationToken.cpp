#include "CancellationToken.h"

bool CancellationToken::IsCanceled() {
	return cancelled;
}

void CancellationToken::Cancel() {
	cancelled = true;
}

void CancellationToken::Reset() {
	cancelled = false;
}