SELECT origin_name
FROM Champions NATURAL JOIN ChampionOrigins
JOIN Origins USING (origin_id)
GROUP BY origin_id
HAVING COUNT(champion_id) >= 3 AND AVG(champion_tier) > (
  SELECT AVG(champion_tier)
  FROM Champions NATURAL JOIN ChampionClasses
  JOIN Classes USING (class_id)
  WHERE class_name = 'Executioner'
)
ORDER BY origin_name;