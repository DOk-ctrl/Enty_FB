#include "Localization.h"

Lang Localization::currentLang = Lang::RU;

std::map<std::string, std::map<Lang, std::string>> Localization::dict = {
	{"settings",			{{Lang::EN, "Settings/Настройки"},		 {Lang::RU, "Settings/Настройки"}}},
	{"fullscreen",			{{Lang::EN, "Fullscreen"},		 {Lang::RU, "Полный экран"}}},
	{"exit",				{{Lang::EN, "Exit to workflow"},			 {Lang::RU, "Выйти из игры"}}},
	{"language",			{{Lang::EN, "Language/Язык"},		 {Lang::RU, "Language/Язык"}}},
	{"save",				{{Lang::EN, "Save"},			 {Lang::RU, "Сохранить"}}},
	{"load",				{{Lang::EN, "Load"},			 {Lang::RU, "Загрузить"}}},
	{"system",				{{Lang::EN, "[ SYSTEM ]"},			 {Lang::RU, "[ СИСТЕМА ]"}}},
	{"grathics",				{{Lang::EN, "Grathics"},			 {Lang::RU, "Графика"}}},
	{"back",				{{Lang::EN, "Back"},			 {Lang::RU, "Назад"}}},
	{"camera",				{{Lang::EN, "Camera"},			 {Lang::RU, "Камера"}}},
	{"resolution",				{{Lang::EN, "Resolution"},			 {Lang::RU, "Разрешение"}}},
	{"vsync",				{{Lang::EN, "V-sync"},			 {Lang::RU, "Вертикальная синхронизация"}}},
	{"off",				{{Lang::EN, "off"},			 {Lang::RU, "Выкл."}}},
	{"antialiasing",				{{Lang::EN, "Antialising"},			 {Lang::RU, "Сглаживание"}}},
	{"texture_filtering",				{{Lang::EN, "Texture filtering"},			 {Lang::RU, "Фильтрация текстур"}}},
	{"on",				{{Lang::EN, "on"},			 {Lang::RU, "Вкл."}}},
	{"texture_quality",				{{Lang::EN, "Texture Quality"},			 {Lang::RU, "Качество текстур"}}},
	{"ultra",				{{Lang::EN, "Ultra"},			 {Lang::RU, "Очень высокое"}}},
	{"high",				{{Lang::EN, "High"},			 {Lang::RU, "Высокое"}}},
	{"medium",				{{Lang::EN, "Medium"},			 {Lang::RU, "Среднее"}}},
	{"low",				{{Lang::EN, "Low"},			 {Lang::RU, "Низкое"}}},
	{"russian",				{{Lang::EN, "Русский"},			 {Lang::RU, "Русский"}}},
	{"start",				{{Lang::EN, "Start game"},			 {Lang::RU, "Начать игру"}}},
	{"emit",				{{Lang::EN, "Emit(Bismark will change)"},			 {Lang::RU, "Подсветка(Бисмарк поменяется)"}}},

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