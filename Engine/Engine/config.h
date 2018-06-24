#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <unordered_map>
#include <list>
#include <regex>


class config
{
private:
	void parse(std::istream& in) {
		static const std::regex comment_regex{ R"x(\s*[;#])x" };
		static const std::regex section_regex{ R"x(\s*\[([^\]]+)\])x" };
		static const std::regex value_regex{ R"x(\s*(\S[^ \t=]*)\s*=\s*((\s?\S+)+)\s*$)x" };
		std::string current_section = "";
		std::smatch pieces;
		for (std::string line; std::getline(in, line);)
		{
			if (line.empty() || std::regex_match(line, pieces, comment_regex)) {
				// skip comment lines and blank lines                    
			}
			else if (std::regex_match(line, pieces, section_regex)) {
				if (pieces.size() == 2) { // exactly one match
					current_section = pieces[1].str();
				}
			}
			else if (std::regex_match(line, pieces, value_regex)) {
				if (pieces.size() == 4) { // exactly enough matches
					inimap[current_section][pieces[1].str()] = pieces[2].str();
				}
			}
		}
	}

	void serialize(std::ostream& out) const {
		for (const auto &heading : inimap) {
			out << "[" << heading.first << "]"<< std::endl;
			for (const auto &kvs : heading.second) {
				out << kvs.first << '=' << kvs.second << std::endl;
			}
			out << std::endl;
		}
	}

	using SectionMap = std::unordered_map<std::string, std::string>;
	using IniMap = std::unordered_map<std::string, SectionMap>;

	IniMap inimap;

public:
	config(std::istream& in) { parse(in); }
	config(std::ostream& out) { serialize(out); }
	typedef IniMap::iterator iterator;
	iterator begin() { return inimap.begin(); }
	iterator end() { return inimap.end(); }
	IniMap GetIniMap() const { return inimap; }

	SectionMap const GetSection(const std::string sectionname) const
	{
		return inimap.at(sectionname);
	}

	std::string const GetValue(const std::string sectionname, const std::string keyname) const
	{
		try
		{
			const auto section = GetSection(sectionname);
			try
			{
				return section.at(keyname);
			}
			catch (std::out_of_range)
			{
				return "";
			}
		}
		catch (std::out_of_range)
		{
			return "";
		}
	}
};

#endif // CONFIG_H_