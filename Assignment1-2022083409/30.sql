WITH class_avg AS (
  SELECT class_id, AVG(champion_tier) as A
  FROM Champions NATURAL JOIN ChampionClasses
  JOIN Classes USING (class_id)
  GROUP BY class_id
)
SELECT champion_name
FROM Champions NATURAL JOIN ChampionClasses
JOIN class_avg USING (class_id)
GROUP BY champion_name
HAVING SUM(CASE WHEN champion_tier < A THEN 0 ELSE 1 END) = 0
