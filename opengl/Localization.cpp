#include "Localization.h"

Lang Localization::currentLang = Lang::RU;

std::map<std::string, std::map<Lang, std::string>> Localization::dict = {
	{"settings",			{{Lang::EN, "Settings"},		 {Lang::RU, u8"Настройки"}}},
	{"fullscreen",			{{Lang::EN, "Fullscreen"},		 {Lang::RU, u8"Полный экран"}}},
	{"exit",				{{Lang::EN, "ABANDON OPERATION"},			 {Lang::RU, u8"Выйти из игры"}}},
	{"language",			{{Lang::EN, "Language"},		 {Lang::RU, u8"Язык"}}},
	{"save",				{{Lang::EN, "Save"},			 {Lang::RU, u8"Сохранить"}}},
	{"load",				{{Lang::EN, "Load"},			 {Lang::RU, u8"Загрузить"}}},
	{"system",				{{Lang::EN, "[ SYSTEM ]"},			 {Lang::RU, u8"[ СИСТЕМА ]"}}},
};

std::string Localization::get(const std::string& key) {
	return dict[key][currentLang];
}

void Localization::setLanguage(Lang lang) {
	currentLang = lang;
}

Lang Localization::getCurrentLang() {
	return currentLang;
}