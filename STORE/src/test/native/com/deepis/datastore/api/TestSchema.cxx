#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef COM_DEEPIS_DB_CHARSET
namespace com { namespace deepis { namespace datastore { namespace api { } } } }

#include "cxx/lang/String.h"
#include "cxx/io/RandomAccessFile.h"
#include "com/deepis/datastore/api/DeepStore.cxx"

using namespace cxx::lang;
using namespace cxx::io;
using namespace com::deepis::db::store::relative::core;

static void test(RandomAccessFile& f, const String& test, longtype expected) {
	f.seek(0);
	f.setLength(0, true);
	f.writeBytes(&test);
	f.seek(0);
	
	longtype term = DeepSchema_v1<inttype>::validate(&f);
	if (term != expected) {
		DEEP_LOG(ERROR, OTHER, "'%s': expected %lld, got %lld\n", test.data(), expected, term);
		f.close();
		File(f.getPath()).clobber();
		abort();
	}
}

int main(int argc, char** argv) {
	static const String marker("<mark/>\n");

	const char* tmpfile = tmpnam(NULL);
	RandomAccessFile f(tmpfile, "rw");

	test(f, marker, 8);
	test(f, " "+marker, 9);
	test(f, "  "+marker, 10);
	test(f, "   "+marker, 11);
	test(f, "    "+marker, 12);
	test(f, "     "+marker, 13);
	test(f, "      "+marker, 14);
	test(f, "       "+marker, 15);
	test(f, "        "+marker, 16);
	test(f, "         "+marker, 17);
	test(f, " "+marker+" ", 9);
	test(f, " "+marker+"  ", 9);
	test(f, " "+marker+"   ", 9);
	test(f, " "+marker+"    ", 9);
	test(f, " "+marker+"     ", 9);
	test(f, " "+marker+"      ", 9);
	test(f, " "+marker+"       ", 9);
	test(f, " "+marker+"        ", 9);
	test(f, " "+marker+"         ", 9);
	test(f, marker+" ", 8);
	test(f, " "+marker+" ", 9);
	test(f, "NO MARKER HERE", -1);
	test(f, marker+marker, 16);
	test(f, "                          "+marker+"         ", 34);

	f.close();
	File(tmpfile).clobber();

	return 0;
}

