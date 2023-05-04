#include "options.h"

Options::Options()
	:comparator(BytewiseComparator()),env(Env::Default())
{
}
