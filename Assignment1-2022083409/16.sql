SELECT champion_name
FROM (Champions NATURAL JOIN ChampionClasses)
LEFT OUTER JOIN Classes Using (class_id)
WHERE class_name IS NULL
ORDER BY champion_name ASC;