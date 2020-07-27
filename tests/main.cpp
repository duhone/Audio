#define DOCTEST_CONFIG_IMPLEMENT
#include <3rdParty/doctest.h>

int main(int argc, char** argv) {
	doctest::Context context;

	context.applyCommandLine(argc, argv);

	int res;
	{
		res = context.run();
	}

	if(context.shouldExit()) return res;

	return res;
}
