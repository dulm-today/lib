#include "test_core.h"
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <locale.h>

void output(const char* msg)
{
	fprintf(stderr, (const char*)msg);
	//fwrite(msg, 1, strlen(msg), stderr);
}

extern "C" {
	void(*g_global_log_output)(const char*) = output;
}

struct TestUnit
{
	const char* name;
	test_main	fn;
	int			total;
	int			error;

	TestUnit(const char* name_, test_main fn_, int total_)
		: name(name_), fn(fn_), total(total_), error(0)
	{}
};


std::vector<TestUnit>*	test_list_ptr = NULL;

static std::vector<TestUnit>* test_unit_instance()
{
	if (test_list_ptr == NULL)
		test_list_ptr = new std::vector<TestUnit>;

	return test_list_ptr;
}

Test::Test(const char* name, test_main fn, int total)
{
	std::vector<TestUnit>& ref = *test_unit_instance();

	ref.push_back(TestUnit(name, fn, total));
}

void TestCoreHead(int total)
{
	const char* head = 
		"*******************************************\n"
		"             TestCore  total:%d\n"
		"\n";
	
	fprintf(stderr, head, total);
	printf(head, total);
}

void TestCoreTail(int total, std::vector<int> errlist)
{
	const char* tail = 
		"\n"
		"All result: [%d/%d]\n"
		"*******************************************\n";

	fprintf(stderr, tail, total - errlist.size(), total);
	printf(tail, total - errlist.size(), total);

	if (errlist.size() > 0){
		std::vector<TestUnit>& ref = *test_unit_instance();
		
		fprintf(stderr, "  error: %d unit\n", errlist.size());
		for (int index = 0; index < errlist.size(); ++index){
			TestUnit& ref_unit = ref[errlist[index]];
			fprintf(stderr, "    %-16s [%d/%d]\n", ref_unit.name, 
					ref_unit.total - ref_unit.error, ref_unit.total);
		}
	}
}

void TestHead(const char* name, int total)
{
	const char* head = 
		"-----------------------------------------\n"
		"  %s  total:%d\n"
		"\n";
	
	fprintf(stderr, head, name, total);
	printf("Testing %-48s", name);
}

void TestTail(int total, int error)
{
	const char* tail = 
		"\n"
		"result: [%d/%d] %s\n"
		"-----------------------------------------\n"
		"\n";

	fprintf(stderr, tail, total-error, total, 
			(error == 0 ? "OK" : "Something Error"));
	printf("[%d/%d] %s\n", total-error, total,
			(error == 0 ? "OK" : "Error"));
}



void report_file_open(int argc, const char** argv)
{	
	const char* file = "test_report.log";
	
	FILE* fd_err = freopen(file, "w", stderr);

	if (NULL == fd_err ){
		printf("freopen %s fail\n", file);
	}
}

void report_file_close()
{
	
}

int main(int argc, const char** argv)
{
	std::vector<TestUnit>& ref = *test_unit_instance();
	std::vector<int> errlist;

	setlocale(LC_ALL,"");

	report_file_open(argc, argv);
	
	TestCoreHead(ref.size());
	
	int index = 0;
	for (;index < ref.size(); ++index){
		int error = 0;
		TestUnit& ref_unit = ref[index];
		
		TestHead(ref_unit.name, ref_unit.total);
		
		if (NULL != ref_unit.fn)
		{
			error = ref_unit.fn();
		}
		else
		{
			error = ref_unit.total;
			fprintf(stderr, " fn == NULL\n");
		}

		ref_unit.error = error;
		
		if (error > 0)
			errlist.push_back(index);

		TestTail(ref_unit.total, error);
	}

	TestCoreTail(ref.size(), errlist);
	report_file_close();

	system("pause");
	
	return 0;
}

