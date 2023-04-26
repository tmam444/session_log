#include "CUnit/Basic.h"
#include "session_log.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

int init_suite2(void)
{
	printf("init suite2");
	return (0);
}

int clean_suite2(void)
{
	printf("clean suite2");
	return (0);
}

void	testPARSING_ERROR_CMD_FILE(void)
{
	DIR				*dir;
    struct dirent	*entry;
    char			*directory = "/home/chulee/session_log/test/cmd_files/"; // 특정 디렉토리 경로를 지정하세요.
	char			*file_data;
	char			file_path[1024];
	error_code		err_code;

    dir = opendir(directory);
	if (!dir)
	{
		perror("opendir");
		return ;
	}
    while ((entry = readdir(dir)) != NULL) {
		snprintf(file_path, sizeof(file_path), "%s/%s", directory, entry->d_name);
		if (!strncmp(entry->d_name, "error", 5))
		{
			err_code = NONE;
			file_data = command_file_read(file_path);
			command_parsing(file_data, &err_code);
			CU_ASSERT(err_code != NONE);
		}
	}
    closedir(dir);
}

void	testPARSING_NORMAL_CMD_FILE(void)
{
	DIR				*dir;
    struct dirent	*entry;
    char			*directory = "/home/chulee/session_log/test/cmd_files/"; // 특정 디렉토리 경로를 지정하세요.
	char			*file_data;
	char			file_path[1024];
	error_code		err_code;

    dir = opendir(directory);
	if (!dir)
	{
		perror("opendir");
		return ;
	}
    while ((entry = readdir(dir)) != NULL) {
		snprintf(file_path, sizeof(file_path), "%s/%s", directory, entry->d_name);
		if (!strncmp(entry->d_name, "normal", 6))
		{
			err_code = NONE;
			file_data = command_file_read(file_path);
			command_parsing(file_data, &err_code);
			CU_ASSERT(err_code == NONE);
		}
	}
    closedir(dir);
}

void	testRAW_DATA_FILE(void)
{
	int							i;
	struct session_simulator	*s_simulator = get_simulator();

	s_simulator->stime = 1681742640;
	s_simulator->log_files = list_push_back(s_simulator->log_files, ntk_strdup("/home/chulee/session_log/test/data_files/two_data_raw.dat.23.45_00"));
	read_thread(s_simulator);
	write_thread(s_simulator);
	for (i = 0; i < 60; i++)
		CU_ASSERT(s_simulator->m_data[CUR].s_data[2][i].internal[35].total_byte == 300);
	printf("total_byte = %llu\n", s_simulator->m_data[CUR].t_data[2][0].internal.total_byte);
	CU_ASSERT(s_simulator->m_data[CUR].t_data[2][0].internal.total_byte == 300);
	printf("log_files = %s\n", (char *)s_simulator->log_files->value);
	list_free(s_simulator->log_files);
	s_simulator->log_files = NULL;
	memset(&s_simulator->m_data[CUR], 0, sizeof(struct minute_data));
}

void	testRAW_DATA_FILE2(void)
{
	int							i;
	struct session_simulator	*s_simulator = get_simulator();

	s_simulator->stime = 1681742640;
	s_simulator->log_files = list_push_back(s_simulator->log_files, ntk_strdup("/home/chulee/session_log/test/data_files/three_data_raw.dat.23.45_00"));
	read_thread(s_simulator);
	write_thread(s_simulator);
	for (i = 0; i < 60; i++)
		CU_ASSERT(s_simulator->m_data[CUR].s_data[2][i].internal[35].total_byte == 300);
	printf("total_byte = %llu\n", s_simulator->m_data[CUR].t_data[2][0].internal.total_byte);
	CU_ASSERT(s_simulator->m_data[CUR].t_data[2][0].internal.total_byte == 500);
	list_free(s_simulator->log_files);
	s_simulator->log_files = NULL;
	memset(&s_simulator->m_data[CUR], 0, sizeof(struct minute_data));
}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main()
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   pSuite = CU_add_suite("Cmd Read Suite_2", init_suite2, clean_suite2);
   CU_add_test(pSuite, "test of cmd_file_read", testPARSING_ERROR_CMD_FILE);
   CU_add_test(pSuite, "test of raw data file", testRAW_DATA_FILE);
   CU_add_test(pSuite, "test of raw data file2", testRAW_DATA_FILE2);

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}

