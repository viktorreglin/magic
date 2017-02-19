# magic
it's magic! TEST

Ziel:
Kreiere und verwalte MTG Decks.

Benutzung:
Per Kommandozeile hat der User hat die Wahl (1) eine neue Sammlung zu erstellen oder (2) eine bestehende zu verwalten (Deck ist eine Liste von Karten, mit denen man spielt).

(1): Neue Sammlung erstellen
     User vergibt einen Namen für die neue Sammlung.
     User gibt eine Kombination aus Kriterien ein, anhand derer eine (englische) Datenbank aller Karten durchsucht wird.
     Beispiel wonach der User suchen könnte: Non-black warrior creatures with converted mana cost between 2 and 5, that have at least one of the strings "whe" or "enters" in their text and that cost less than 1 €.
     User kann die zutreffenden Karten einzeln druchgehen und zur Sammlung hinzufügen, zum Bereich All Cards.
     Sammlungen bestehen aus 3 Teilen:
         All Cards - alle Karten, die potentiell im Deck verwendet werden.
         Main - das eigentlich Deck. Enthält die wirklich benutzten Karten inklusive jeweiliger Anzahl. Nur Karten aus All Cards können verwendet werden.
         Sideboard - zwischen Spielen können Karten von Sideboard und Main getauscht werden.
     Nachdem Karten zu All Cards hinzugefügt wurden, können sie zu Main oder Sideboard hinzugefügt werden.
     
(2): Bestehende Sammlung verwalten.
     User wählt aus Liste aller vorhandenen Sammlungen aus.
     User kann beliebige Änderungen an einer Sammlung vornehmen.
     
     
Was soll mit einer Sammlung möglich sein?
     Angucken.
     Karten hinzufügen, löschen, Anzahl in Main oder Sideboard ändern.
     Statistiken der drei Bereiche einzeln abrufen.
     Main und Sideboard müssen irgendiwe in die Zwischenablage gebracht werden können (evtl. über Textdatei), in einem bestimmten Format.
        Beispiel:
        17 Forest
        
        
Link zur Kartendatenbank: https://mtgjson.com/json/AllCards.json.zip
Kartenpreise API: https://www.mkmapi.eu/ws/documentation/API_Main_Page
