/*
 * utils.hh
 *
 */

#ifndef SRC_UTILS_HH_
#define SRC_UTILS_HH_
#include <string>
#include <vector>

class Utils {
public:
	Utils();
	virtual ~Utils();

	static std::string& ltrim(std::string &s);
	static std::string& rtrim(std::string &s);
	static std::string& trim(std::string &s);
	static void split(std::string &s, char c, std::vector<std::string>& out);
	static std::vector<std::string> split(std::string &s, char c);
	static std::string join(std::vector<std::string> arr, char c);
	static uint64_t get_timestamp_ms();
};

#endif /* SRC_UTILS_HH_ */
