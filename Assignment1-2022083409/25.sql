WITH count_classes AS (
  SELECT champion_id, champion_name, COUNT(class_id) AS class_count
  FROM Champions NATURAL JOIN ChampionClasses
  JOIN Classes USING (class_id)
  GROUP BY champion_id
),
origin_rank AS (
  SELECT champion_name, class_count, RANK() OVER (
    PARTITION BY origin_id ORDER BY class_count DESC) AS R
  FROM count_classes NATURAL JOIN ChampionOrigins
  JOIN Origins USING (origin_id)
)
SELECT champion_name, class_count
FROM origin_rank
WHERE R = 1
ORDER BY champion_name;