WITH highest_tier AS (
  SELECT champion_name, champion_tier, DENSE_RANK() OVER (
    PARTITION BY class_id ORDER BY champion_tier ASC) AS R
  FROM Champions NATURAL JOIN ChampionClasses
  JOIN Classes using (class_id)
)
SELECT champion_name, champion_tier
FROM highest_tier
WHERE R = 2
ORDER BY champion_name;
