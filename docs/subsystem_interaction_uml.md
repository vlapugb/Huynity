# UML взаимодействия подсистем Huynity

Диаграмма последовательности после [архитектурного ревью](architecture_review.md).
Показывает создание объекта и жизненный цикл асинхронного задания кадра.
Object Library представлена как участник синхронных вызовов в главном потоке,
а не актор. Renderer Port — адаптер транспорта и очереди завершений.

[Исходник PlantUML](subsystem_interaction.puml). Для отображения блока требуется
поддержка PlantUML в Markdown-просмотрщике. Локальный PlantUML не установлен;
SVG не сгенерирован, визуальная проверка не выполнена.

Синхронные вызовы показаны `->`, возвраты `-->`, асинхронные сообщения `->>`.
Фрагменты `alt`, `opt`, `loop` и `par` задают условия, повторения и конкурентное
выполнение. Главный цикл повторяется; после Empty порт опрашивается на следующем
проходе. Фрагмент `par` иллюстрирует перекрытие работы потоков и не вводит барьер:
главный поток не ждёт завершения растеризации.

```plantuml
@startuml
title Huynity — создание объекта и асинхронное построение кадра
hide footbox
autonumber
skinparam sequenceMessageAlign center

box "Главный поток" #EEF4FF
participant "UI / Editor" as UI
participant "Orchestrator" as O
participant "Core\nвключая Render Preparation" as C
participant "Object Library\nобычный C++ API" as L
participant "Renderer Port\nадаптер транспорта" as P
participant "Window / Presentation" as W
end box
box "Рабочий исполнитель" #F0FFF0
participant "Renderer actor" as R
end box

UI -> O : C++: submit(CreateObject, requestId)
activate O
O --> UI : SubmitResult: Accepted / Rejected
deactivate O

group Фазы главного цикла до отправки кадра
O -> W : C++: pollEvents()
W --> O : input / resize / close
O -> O : обработать resize; обновить surfaceGeneration\nпри close перейти в Stopping
opt Running и есть принятая команда
O -> C : C++: apply(CreateObject)
activate C
C -> L : C++: создать геометрический объект
L --> C : объект / ошибка
C -> C : при успехе сохранить экземпляр и назначить ID
C --> O : CommandResult(requestId, objectId / error)
deactivate C
O -> O : сохранить результат для UI
end
UI -> O : C++: takeResults()
O --> UI : список CommandResult
loop Running и накоплен шаг h; не более N обновлений
O -> C : C++: update(h)
C --> O : завершено
end
end

opt Running, Idle и ненулевой размер окна
O -> C : C++: prepareFrame(FrameSpec)
activate C
C -> L : C++: операции подготовки геометрии
L --> C : примитивы / ошибка
C --> O : RenderSnapshot / PrepareError
deactivate C
alt Ошибка подготовки
O -> O : сохранить ошибку; остаться Idle
else Снимок подготовлен
O -> P : C++: trySubmit(RenderFrame(frameId, generation, snapshot))
activate P
alt Транспорт принимает задание
P ->> R : async mailbox: RenderFrame\nпередача владения снимком
P --> O : Accepted
O -> O : pendingFrame = frameId; состояние Busy
else Транспорт закрыт или не готов
P --> O : Rejected\nвладение снимком остаётся у вызывающего
O -> O : остаться Idle; сохранить ошибку
end
deactivate P
end
end

opt Задание было принято
par Главный поток не ждёт Renderer
O -> W : C++: pollEvents()
W --> O : input / resize / close
O -> O : обновить generation / состояние остановки
opt Running
O -> C : C++: применить следующие команды / update(h)
C --> O : результаты
end
else Рабочий исполнитель
R -> R : rasterize(snapshot) в собственный framebuffer
alt Успех
R ->> P : очередь завершений: FrameReady(frameId, generation, buffer)
else Ошибка задания
R ->> P : очередь завершений: FrameFailed(frameId, error)
end
end

O -> P : C++: pollCompletion()
P --> O : Empty / FrameReady / FrameFailed / RendererDown
alt Нет завершения
O -> O : сохранить Busy; следующий проход цикла
else RendererDown
O -> O : состояние Failed; запретить новые кадры\nдождаться подтверждения окончания исполнения
else frameId не совпадает с pendingFrame
O -> O : отклонить завершение; не менять текущий слот
else FrameFailed текущего задания
O -> O : очистить pendingFrame; сохранить ошибку
else FrameReady текущего задания
O -> O : очистить pendingFrame
alt Running и generation совпадает
O -> W : C++: present(buffer)
W --> O : PresentResult; ссылка на buffer не удерживается
else Stopping или старое поколение окна
O -> O : освободить buffer без показа
end
end
end

note over O,R
Busy сохраняется до обработки завершения, даже если вычисление уже закончено.
RendererDown публикуется монитором исполнения через порт, а не умирающим актором.
Публикация завершения не вызывает UI или Core из рабочего потока.
end note

legend bottom
  -> синхронный C++ вызов
  --> возврат из вызова
  ->> асинхронная отправка через транспорт
  Object Library не имеет собственного потока и почтового ящика.
  Framebuffer — данные; его владение передаётся с FrameReady.
endlegend
@enduml
```
