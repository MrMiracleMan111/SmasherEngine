#include "UUID.h"

namespace Smasher {
	UUID UUID::GetUUID() {
		++s_count;
		return UUID{ s_count };
	}
}