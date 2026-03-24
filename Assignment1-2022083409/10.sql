SELECT DISTINCT class_name
FROM Champions NATURAL JOIN ChampionClasses NATURAL JOIN Classes
WHERE champion_tier = 4
ORDER BY class_name ASC;