#include "EasyRecordQuery.h"
#include <boost/utility/string_ref.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include "EasyRecordSession.h"

EasyRecordQuery::EasyRecordQuery(const char* path, const char* name)
: path_(path)
, name_(name)
{
}

EasyRecordQuery::~EasyRecordQuery(void)
{
}

bool EasyRecordQuery::List(const char *begin, const char *end, vector<string> *records)
{

	GetM3U8(begin, end, records);

	return !records->empty();
}

void EasyRecordQuery::GetAllM3U8(std::vector<string> *records)
{
	namespace fs = boost::filesystem;

	fs::path fullpath = fs::path(EasyRecordSession::sLocalRecordPath) / name_;

    if (!fs::exists(fullpath))
    {
        return;
    }
    fs::recursive_directory_iterator end_iter;
    for (fs::recursive_directory_iterator iter(fullpath); iter != end_iter; iter++)
    {
        try
        {			//cout << iter->path().string() << endl;
			if (!fs::is_directory(*iter))
            {
				string file = iter->path().string();
				boost::string_ref m3u8_file(file);
				//cout << "string_ref: " << m3u8_file.data() << endl;
				
				if (m3u8_file.ends_with(".m3u8"))
                {
                    records->push_back(m3u8_file.data());                          
                }
            }
        }
        catch (const std::exception & ex)
        {
            std::cerr << ex.what() << std::endl;

            continue;
        }
    }
}

void EasyRecordQuery::GetM3U8(const char* begin, const char* end, vector<string>* records)
{
	namespace fs = boost::filesystem;

	fs::path fullpath = fs::path(EasyRecordSession::sLocalRecordPath) / name_;

    if (!fs::exists(fullpath))
    {
        return;
    }
    fs::recursive_directory_iterator end_iter;
    for (fs::recursive_directory_iterator iter(fullpath); iter != end_iter; iter++)
    {
        try
        {			//cout << iter->path().string() << endl;
			if (!fs::is_directory(*iter))
            {
				string file = iter->path().string();
				boost::string_ref m3u8_file(file);
				//cout << "string_ref: " << m3u8_file.data() << endl;
				
				if (m3u8_file.ends_with(".m3u8"))
                {
                    char split = '/';
#ifdef _WIN32
					split = '\\';
#endif

					int pos = m3u8_file.find_last_of(split);
					boost::string_ref file_time = m3u8_file.substr(pos - 14, 14); // /20151123114500/*.m3u8
					//cout << "file_time: " << file_time << " condition[" << begin << " - " << end << "]" <<endl;
					if(file_time > end || file_time < begin)
					{
						cout << "skip m3u8 file: " << m3u8_file.data() << " condition[" << begin << " - " << end << "]" << endl;
						continue;
					}
					records->push_back(m3u8_file.data());                          
                }
            }
        }
        catch (const std::exception & ex)
        {
            std::cerr << ex.what() << std::endl;

            continue;
        }
    }
}

time_t EasyRecordQuery::StringToTime(const string time_str)
{
	if(time_str.length() != 14)
	{
		cout << "time string format error: " << time_str << endl;
		return 0;
	}

	struct tm local;

	time_t now = time(NULL);
#ifdef _WIN32
	localtime_s(&local, &now);
#else
	localtime_r(&now, &local);
#endif
	sscanf(time_str.c_str(), "%04d%02d%02d%02d%02d%02d", &local.tm_year, &local.tm_mon, &local.tm_mday, \
			&local.tm_hour, &local.tm_min, &local.tm_sec);
	local.tm_year -= 1900;
	local.tm_mon -= 1;

	return mktime(&local);
}