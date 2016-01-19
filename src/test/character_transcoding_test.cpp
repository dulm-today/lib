#include "character_transcoding.h"
#include "test_core.h"

int character_transcoding_test_main()
{
	int error = 0;

	const char* str = "好好学习天天向上";
	const char* wstr = "好好学习天天向上";
	const char* utf8 = "濂藉ソ瀛︿範澶╁ぉ鍚戜笂";
	char* pstr = NULL;
	wchar_t* wpstr = NULL;
	size_t size = 0;

	std::exwstring a = conv_mbs2wcs(std::exstring(str), -1, CP_ACP);
	std::exstring b = conv_wcs2mbs(std::exwstring(wstr), -1, CP_ACP);
	std::exstring c = convert(std::exstring(utf8), -1, CP_UTF8, CP_ACP);

	fprintf(stderr, "1. %ls\n", a.c_str());
	fprintf(stderr, "2. %s\n", b.c_str());
	fprintf(stderr, "3. %s\n", c.c_str());

	return error;
}

Test test_item_character_transcoding("character_transcoding", character_transcoding_test_main, 1);
