Idea rozwiązania części C:

Serwer w momencie dołączania do grupy:
czeka na dostęp do sekcji krytycznej.
Dla każdego pliku, który posiada, wysyła pakiet 'DEL' z tą nazwą pliku na adres grupowy, aby pozostałe serwery mogły go usunąć

W momencie otrzymania pakietu 'ADD' od klienta:
przyjmuje plik od klienta (o ile ma miejsce), wpisując go do listy posiadanych plików jeszcze przed zakończeniem pobierania
czeka na dostęp do sekcji krytycznej
jeśli w trakcie czekania na dostęp do sekcji krytycznej otrzyma pakiet 'DEL' z nazwą pliku, który pobiera, to przerywa pobieranie i usuwa dotychczas pobrane dane.
gdy uzyska dostęp do sekcji krytycznej, wysyła pakiet 'DEL' z nazwą otrzymanego pliku, aby pozostałe serwery mogły go usunąć.

W momencie otrzymania pakietu 'fetch' od klienta:
jeśli posiada plik to go wysyła, jeśli nie posiada to zachowuje się zgodnie z częścią A (nie odpowiada na pakiet)

W momencie, gdy serwer chce odłączyć się od grupy:
czeka na dostęp do sekcji krytycznej
wysyła do serwerów zmodyfikowany pakiet 'HELLO' na którą odpowiadają zarówno aktualnie wolnym miejscem jak i całkowitym miejscem.
dla każdego pliku, który posiada, zaczynając od największych:
jeśli jest serwer, który posiada tyle miejsca, ile potrzeba, to wysyła go do tego serwera i przechodzi do następnego pliku.
w przeciwnym razie wybiera serwer, o jak największym wolnym miejscu i całkowitym miejscu na tyle dużym, że zmieści cały plik.
wysyła do serwera pakiet 'SPLIT' z parametrem param równym wielkości pliku.
Serwer otrzymujący pakiet 'SPLIT' reaguje na ten pakiet tak, że zaczyna rozsyłać swoje pliki do pozostałych serwerów
(jeśli jakiś plik jest większy niż największa wolna pojemność serwera, to go zostawia u siebie) i usuwa je ze swojego dysku, do momentu,
gdy ma wystarczająco miejsca na przyjęcie pliku.
Jeśli uda mu się zwolnić wystarczająco miejsca, to wysyła serwerowi od którego odebrał 'SPLIT' informację o tym, że jest gotowy na przyjęcie pliku, a następnie go przyjmuje.
W przeciwnym razie wysyła informację, że nie jest w stanie przyjąć pliku. W takim wypadku ten plik przepada, a serwer odłączający się kontynuuje próbę przesłania kolejnego z kolei pliku.

Dostęp do sekcji krytycznej można zrealizować przy pomocy algorytmu Nielsena-Misuno.

