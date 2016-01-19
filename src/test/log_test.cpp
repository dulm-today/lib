#include "log.h"
#include "test_core.h"

static LOG_V_MODULE(log_test_cbv, "log_test", g_global_log_output, 1);

int log_test_main()
{
	LOG_CB_V(log_test_cbv, LOG_DEBUG, "1.1.1_%s", "haha");
	LOG_CB_V(log_test_cbv, LOG_MSG,   "1.1.2_%s", "haha");
	LOG_CB_V(log_test_cbv, LOG_WARN,  "1.1.3_%s", "haha");
	log_level_set(LOG_ERR|LOG_DEBUG_ERR);
	LOG_CB_V(log_test_cbv, LOG_MSG,   "1.2.2");
	LOG_CB_V(log_test_cbv, LOG_WARN,  "1.2.3");
	LOG_CB_V(log_test_cbv, LOG_ERR,   "1.2.4");
	LOG_CB_V(log_test_cbv, LOG_DEBUG_ERR, "1.2.5");
	LOG_CB_V(log_test_cbv, LOG_DEBUG|LOG_DEBUG_ERR, "1.2.6");
	//log_format_set("%time%_(%6level%)_%module%_%func% %aa% %% %data% %data%   aa  ");
	
	LOG_CB_V(log_test_cbv, LOG_ERR,   "1.3.1_%d_%s", 10, "world");
	LOG_CB_V(log_test_cbv, LOG_ERR,   "1.3.2_%d_%s", 10, "hello");
	LOG_CB_V(log_test_cbv, 6,   "1.3.2_%d_%s", 10, "hello");
	
	return 0;
}


Test log_test_item("log", log_test_main, 1);

