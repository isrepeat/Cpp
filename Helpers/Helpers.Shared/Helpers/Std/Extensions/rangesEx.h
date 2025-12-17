#pragma once
#include "Helpers/common.h"
#include "rangesEx/ranges_flatten_tree.h"
#include "rangesEx/ranges_drop_last.h"
#include "rangesEx/ranges_concat.h"
#include "rangesEx/ranges_to.h"

//
//  ░ Range adaptor pattern (C++23-стиль)
//  ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
//
//  Мы используем единый паттерн для расширений std::ranges::views:
//
//  • <name>_closure  — «замыкание-адаптер»: хранит параметры вызова (если есть) и
//    объявляет friend-оператор `operator|` (hidden friend idiom).
//    Hidden friend даёт доступ к приватным данным closure и участвует в ADL,
//    поэтому выражение `range | <name>(...)` корректно находит нужный оператор.
//
//  • <name>_fn       — функциональный объект с реализацией операции над range.
//    Его удобно иметь, когда есть внутренняя логика помимо самого «пайпинга»
//    (например, вычисление countToTake в drop_last_fn). В простых случаях (как to)
//    отдельный *_fn не обязателен.
//
//  • Публичная фабрика <name>(...) возвращает <name>_closure.
//
//  Hidden friend idiom:
//  - Объявляем operator| прямо внутри <name>_closure как friend.
//  - Он остаётся "свободной" функцией (не членом),
//    но имеет доступ к приватным данным и подхватывается ADL
//    (она объявлена внутри класса, и потому компилятор «привязывает» её 
//    к тому же namespace, где объявлен класс).