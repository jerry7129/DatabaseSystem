WITH tier_list AS(
  SELECT champion_name, RANK() OVER (ORDER BY champion_tier ASC) AS champion_rank
  FROM (Champions NATURAL JOIN ChampionClasses)
  JOIN Classes USING (class_id)
  WHERE class_name = 'Duelist'
)
SELECT champion_name
FROM tier_list
WHERE champion_rank = 1
ORDER BY champion_name ASC;