WITH highest_tier AS (
  SELECT champion_id, DENSE_RANK() OVER (
    PARTITION BY class_id ORDER BY champion_tier ASC) AS R
  FROM Champions NATURAL JOIN ChampionClasses
  JOIN Classes using (class_id)
)
SELECT champion_id
FROM highest_tier
WHERE R = 1
ORDER BY champion_id;
