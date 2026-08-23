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
	{"1.1",				{{Lang::EN, "Agnes is saved!\n\n"},			 {Lang::RU, "Агнесс спасена!\n\n"}}},
	{"2.1",				{{Lang::EN, "Thanks for playing!!!\n"},			 {Lang::RU, "Спасибо за прохождение!!!\n"}}},
	{"3.1",				{{Lang::EN, "Please leave any feedback or comments at"},			 {Lang::RU, "Оставляйте любой фидбек и отзывы на "}}},
	{"goal",				{{Lang::EN, "Mission"},			 {Lang::RU, "Цель "}}},
	{"goalT",				{{Lang::EN, "Mission complete"},			 {Lang::RU, "Цель достигнута "}}},
	{"email",				{{Lang::EN, "HotBB62@hotmail.com"},			 {Lang::RU, "HotBB62@hotmail.com"}}},
	{"4.1",				{{Lang::EN, "   Enterprise!\n"},			 {Lang::RU, "   Энтерпрайз!\n "}}},
	{"5.1",				{{Lang::EN, "    Agness is in danger. Bismarck has lost her mind"},			 {Lang::RU, "   Агнесс в опасности. Бисмарк потеряла рассудок"}}},
	{"6.1",				{{Lang::EN, ", and her mind has been taken over by the sirens!"},			 {Lang::RU, ", и ее разум был подчинен сиренам! "}}},
	{"7.1",				{{Lang::EN, "   Save Agness, don't forget how we used"},			 {Lang::RU, "   Спаси Агнесс, не забывай как мы с тобой "}}},
	{"8.1",				{{Lang::EN, "to train together:"},			 {Lang::RU, "проводили наши тренировки: "}}},
	{"9.1",				{{Lang::EN, "  - Your G-key attack has an 80 percent chance"},			 {Lang::RU, " -  Твоя атака на клавишу G имеет шанс 80"}}},
	{"10.1",				{{Lang::EN, "to deal double damage."},			 {Lang::RU, "процентов нанести двойной урон."}}},
	{"11.1",				{{Lang::EN, "  - The rest of the keys F, LMB, C inflict "},			 {Lang::RU, "  - Остальные на клавиши F, ЛКМ, С наносят "}}},
	{"12.1",				{{Lang::EN, "base damage\n "},			 {Lang::RU, "свой базовый урон\n  "}}},
	{"13.1",				{{Lang::EN, "Good luck. I'm counting on you. "},			 {Lang::RU, "Удачи. Я рассчитываю на тебя.  "}}},
	{"14.1",				{{Lang::EN, "Defeat Bismarck to free Agnes at Stonehenge. "},			 {Lang::RU, "Победите Бисмарк, чтобы освободить Агнесс "}}},
	{"15.1",				{{Lang::EN, "Esc for control assistance  "},			 {Lang::RU, "Esc для помощи в управлении  "}}},
	{"feedback",				{{Lang::EN, "ANY FEEDBACK"},			 {Lang::RU, "ОСТАВЬТЕ ОТЗЫВ"}}},
	{"letter",				{{Lang::EN, " Letter from the commander\n "},			 {Lang::RU, " Письмо от командира\n  "}}},
	{"emit",				{{Lang::EN, "Emit(Bismark will change)"},			 {Lang::RU, "Подсветка(Бисмарк поменяется)"}}},
	{"1",				{{Lang::EN, "Yes, I used AI, I confess, I'm guilty, oh so guilty, of using Cloud once every two days because its context is clogged. Where the AI​​really helped:"},			 {Lang::RU, "Да я использовал ИИ, каюсь, я грешен, ох как же грешен что использовал клауд раз в 2 дня, потому что у него контекст забит. Где реально помог ии: "}}},
	{"2",				{{Lang::EN, "debugging kilometer-long logs, implementing template solutions that can take weeks during regular development, writing mathematics that is described in"},			 {Lang::RU, "дебаггинг километровых логов, внедрение шаблонных решений на которые могут уходить недели при обычной разработке, написание математики которая расписана в "}}},
	{"3",				{{Lang::EN, "some reports, articles in Chinese, Japanese and other languages​​​​different from English, even in English, but about which (reports/methods) you say nothing"},			 {Lang::RU, "каких-то докладов, статьях на китайских, японских и других языках отличных от английского, да даже на англ. , но о которых(докладах/методов) ты ничего не "}}},
	{"4",				{{Lang::EN, "You know, the ones that contain the correct algorithm for conditional shaders. You could make an argument here that you just need to read books, well, I"},			 {Lang::RU, "знаешь в которых заложен правильный алгоритм для условных шейдеров. Можно отметить здесь как бы аргумент , что нужно просто читать книги, ну я "}}},
	{"5",				{{Lang::EN, "I read 10, and not a single book, not a single one, wrote about cubic skyboxes, IBL, spherical skyboxes, or parallax for skyboxes. Just, well, that's it."},			 {Lang::RU, "прочитал, 10, ни в одной книге, ни в одной не было написано про кубический скайбокс, IBL , сферический скайбокс, параллакс для скайбокса. Просто ну вот "}}},
	{"6",				{{Lang::EN, "I read a book about Cmake, but it's written very basicly, especially, the only thing it was useful for was Cpack, although"},			 {Lang::RU, "условно я прочитал книгу про Cmake ну она просто написана очень базово особенно, ну единственная для чего она пригодилась это для Cpack, хотя "}}},
	{"7",				{{Lang::EN, "The solution presented there just doesn't work, or, say, a book about GLSL: well, yes, it describes the Blinn Phong formula and others, but nothing about IBL. So,"},			 {Lang::RU, "представленное там решение ну не работает, или условно книга про GLSL: ну да там про формулу блинна фонга и другие расписаны, но про IBL ничего. Так со "}}},
	{"8",				{{Lang::EN, "To everyone: the books simply give basic examples, in which there is nothing complicated or advanced."},			 {Lang::RU, "всем: в книгах просто дается базовые примеры, в которых ну ничего сложного нет, продвинутого. "}}},
	{"9",				{{Lang::EN, "And regarding the number of tokens or requests, well, a cloud is once every two days, or 5 times without context. That's very little. Gemini has almost unlimited freebies."},			 {Lang::RU, "Да и насчет количества токенов или запросов, ну клауд один раз в двое суток, или 5 раз без контекста. Это очень мало. Да у гемини почти безлимит на фри "}}},
	{"10",				{{Lang::EN, "version, but he is not able to write complex mathematics even in pro mode."},			 {Lang::RU, "версии, но он не способен писать сложную математику даже в про режиме."}}},
	{"11",				{{Lang::EN, "As for the context, well, there is Imgui, which has a very good GUI, so chic that with some effort it can easily be of high quality."},			 {Lang::RU, "Насчет контекста, да вот условно есть Imgui, который имеет очень хороший GUI, шикарный настолько, что ну с определенными усилиями спокойно качественный "}}},
	{"12",				{{Lang::EN, "You can draw a UI. I have my OpenGL engine, which doesn't have crossfade, but the AI ​​thinks otherwise. Well, it doesn't work for me, and even knowing the architecture, they can't do it for me."},			 {Lang::RU, "UI можно нарисовать. А есть мой opengl движок где нет кроссфейда, но ИИ считают иначе, ну не работает он у меня и они даже ну зная архитектуру не могут мне "}}},
	{"13",				{{Lang::EN, "I couldn't help, but I only got it working by writing the architecture myself. It all depends on the quality of the documentation. If it's outdated, AI won't help, so I'll definitely have to do it myself."},			 {Lang::RU, "помочь, только сам написав архитектуру у меня всё заработало. Всё зависит от качества документации, если устарело ну ИИ не поможет, определенно самому "}}},
	{"14",				{{Lang::EN, "I need to write. Blender is also heavily affected by this; it's easier to watch three-minute guides than to read a dissertation on how to add bones."},			 {Lang::RU, "надо писать. Блендер тоже это сильно касается, вот проще трехминутные гайды посмотреть, чем диссертационную работу читать как там кости добавить "}}},
	{"15",				{{Lang::EN, "That's right, it (the AI) says, well, make the chest bone a little smaller, I'm like, how much smaller, and this happens all the time."},			 {Lang::RU, "правильно, оно же(ИИ) говорит ну сделай чуть меньше условно кость груди, я такой а насколько меньше, и это сплошь и рядом. "}}},
	{"16",				{{Lang::EN, "And so when I was switching to the 20th version of the standard, I encountered something like u8 doesn't understand, well, that is, I need to read the 20th standard to understand what the problem is,"},			 {Lang::RU, "И вот я когда переходил на 20 версию стандарта, столкнулся с типа u8 не понимает, ну то есть мне 20 стандарт читать надо чтобы понять в чем проблема, в "}}},
	{"17",				{{Lang::EN, "What interesting book says how to solve this problem, maybe ask a question on the forum and wait three days, well, why, there is AI, and it gives idiotic answers"},			 {Lang::RU, "какой интересно книге написано как решить проблему с этим, может задать вопрос на форуме и ждать трое суток, ну зачем, есть ИИ, да и он дает дебильные "}}},
	{"18",				{{Lang::EN, "Advice like, Well, I deleted the u8, you think it worked ? The only thing that helped was the Cyrillic flags in Cmake. Well, where does it say that? Besides,"},			 {Lang::RU, "советы по типу, ну уюери u8, ну убрал, вы думаете оно завелось, хер там плавал. Помогли только флаги в Cmake на кириллицу, ну это же где написано. Тем более "}}},
	{"19",				{{Lang::EN, "Blender, you have to do everything manually. It's interesting. I tried an AI generator once, but it was terrible. It's all paid, low quality, and all the consequences that come with it, like skill degradation, etc."},			 {Lang::RU, "блендер, только ручками всё делать, вообще интересно я попробовал один раз ИИ генератор, ну это ужас, это всё платно, некачественно и все вытекающие, по типу ну деградация навыков и т.д."}}},
	{"20",				{{Lang::EN, "It's very interesting to watch videos on YouTube that talk about how Cloud has replaced programmers or how applications are written through"},			 {Lang::RU, "Очень интересно смотреть ролики на ютубе рассказывающие про то, что клауд заменил программистов или там написание приложений через "}}},
	{"21",				{{Lang::EN, "writing documentation, or through AI agents, it's so interesting, I literally go into the cloud, I tell him, well, I don't remember there, my head hurts, in short, he tells me"},			 {Lang::RU, "написание документации, или  через ИИ агентов, так интересно, буквально я захожу в клауд, я ему говорю ну это там я уже не помню голова болит, короче он мне "}}},
	{"22",				{{Lang::EN, "I wrote some crap code, I didn't even understand what the hell it was, I sent Gemini and he told me, well, Claude wrote some crap, here's the normal version, do you think I need Gemini?"},			 {Lang::RU, "написал дерьмо код, я сам не понял че это за хрень, скидываю гемини он мне говорит ну клод какую то шляпу написал, вот нормальная версия, вы думаете мне гемини "}}},
	{"23",				{{Lang::EN, "Well written, well, I just insert this code and run it, well, the computer system crashes, great code is written by AI, I agree 300%. Of course, very"},			 {Lang::RU, "нормально написал, ну просто я вставляю этот код запускаю, ну система компьютера крашится, классный код пишутт ИИ, согласен на все 300%. Конечно. Очень "}}},
	{"24",				{{Lang::EN, "It's nice to work with tools when they're blocked in your region, and even nicer when the bypass doesn't work."},			 {Lang::RU, "приятно работать с интрументами, когда для они заблокированы в твоем регионе, и еще приятнее когда средство обхода не работает."}}},
	{"25",				{{Lang::EN, "5 months, $0, 0 support, lost all my friends, ruined relationships. The only help I got was someone recommending SAI 2 instead."},			 {Lang::RU, "5 месяцев, 0$, 0 поддержки, минус все друзья, испорченные отношения. Единственную помощь я получил это мне порекомендовали использовать SAI 2 а не "}}},
	{"26",				{{Lang::EN, "Paint, and they offered me some help writing notes. You might say, What kind of notes ?  Well, it's for the game's lore. You don't get it, don't laugh. So do I."},			 {Lang::RU, "paint, и мне предложили типа ну помощь в плане писать записки. Вы скажете что за записки, ну это для лора, для лора игры, не поняли , не смеетесь, я вот тоже "}}},
	{"27",				{{Lang::EN, "I was shocked by such help. If we go into detail, well, five months. Well, the first month was pure enthusiasm, the second month was worse, June was a complete nightmare."},			 {Lang::RU, "охренел от такой помощи. Если уж говорить подробно ну 5 месяцев, ну первый месяц чистый на энтузиазме прошёл, второй месяц хуже, июнь это вообще ужас был, "}}},
	{"28",				{{Lang::EN, "In July I allowed myself 2 days off a week, and in August without any days off, but now a maximum of 1.5-3 hours of work, that's the limit, by the fifth month it's really hard"},			 {Lang::RU, "июль я позволял себе по 2 выходных в неделю, ну и август без выходных, по уже ну максимум 1.5-3 часа работы, это предел, на пятый месяц ну очень тяжело "}}},
	{"29",				{{Lang::EN, "No days off. Yes, the first month, day and night, but they'll still distract you, no one will ever offer commensurate help, I'd really like that."},			 {Lang::RU, "без выходных. Да первый месяц днями и ночами, но всё равно будут отвлекать, никто вообще никогда не предложит соизмеримую помощь, я бы очень хотел чтобы "}}},
	{"30",				{{Lang::EN, "They laid out the images for me, or searched for deep sounds, but this takes time and effort. You'll say, well, just take a rest, what's the problem? Really, what's the problem when"},			 {Lang::RU, "за меня верстали картинки, или искали звуки глубокие, но это нужно время и силы. Вы скажете ну так отдохни в чем проблема, правда в чем же проблема, когда "}}},
	{"31",				{{Lang::EN, "Once school starts there won't be time for it, and any day of delay is a waste of time, you can't make up for it with anything, it's better not to work at all than not to work at all."},			 {Lang::RU, "начнётся учеба будет не до этого , а любой день промедления это ну потеря, ну время не восполнить ничем, лучше никак не работать, чем вообще не работать. "}}},
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