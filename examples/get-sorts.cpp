#include <iostream>
#include <sparc2cpp.hpp>

using namespace std;
using namespace sp;

int main(int argc, char** argv)
{
	string sparcFile("/home/zero/Dropbox/Tesis/SPARC-Learn/ch8-mod-dyn-dom/blocks.sp");
	DomainSort ds;
	ds.loadFile(sparcFile);

	return 0;
}
