#pragma once
#include <string>
#include <map>
#include <vector>

enum class Lang { EN, RU };

class Localization {
public:
	// Статический метод, чтобы можно было вызвать из любого места: 
	// Localization::get("exit")
	static std::string get(const std::string& key);

	static void setLanguage(Lang lang);
	static Lang getCurrentLang();

private:
	static Lang currentLang;
	static std::map<std::string, std::map<Lang, std::string>> dict;
};

