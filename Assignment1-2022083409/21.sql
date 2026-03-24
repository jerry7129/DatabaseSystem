SELECT champion_name
FROM Champions NATURAL JOIN ChampionClasses
JOIN Classes USING (class_id)
WHERE class_name IN (
  SELECT class_name
  FROM Champions NATURAL JOIN ChampionClasses
  JOIN Classes USING (class_id)
  WHERE champion_name = 'Aatrox'
) AND champion_name != 'Aatrox'
ORDER BY champion_name ASC;