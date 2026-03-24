SELECT class_name
FROM Champions NATURAL JOIN ChampionClasses NATURAL JOIN Classes
WHERE champion_name = 'Gwen'
ORDER BY class_name ASC;