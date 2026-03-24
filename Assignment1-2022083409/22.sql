WITH class_concat AS (
  SELECT champion_name, GROUP_CONCAT(class_name ORDER BY class_name ASC) AS GC
  FROM Champions NATURAL JOIN ChampionClasses
  JOIN Classes USING (class_id)
  GROUP BY champion_id
)
SELECT champion_name
FROM class_concat
WHERE GC = (
  SELECT GROUP_CONCAT(class_name ORDER BY class_name ASC)
  FROM Champions NATURAL JOIN ChampionClasses
  JOIN CLasses USING (class_id)
  WHERE champion_name = 'Jhin'
  GROUP BY champion_id
) AND champion_name != 'Jhin'
