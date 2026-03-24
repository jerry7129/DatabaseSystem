SELECT class_name
FROM (Champions NATURAL JOIN ChampionClasses) 
JOIN Classes USING (class_id)
WHERE champion_name LIKE 'J%'
ORDER BY champion_name ASC;
