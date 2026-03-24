WITH champion_rank AS (
  SELECT champion_id, champion_name, champion_tier, RANK() OVER (
    PARTITION BY origin_id ORDER BY champion_tier) AS R
  FROM Champions NATURAL JOIN ChampionOrigins
  JOIN Origins USING (origin_id)
),
high_tier_champ AS (
  SELECT champion_id, champion_name, champion_tier
  FROM champion_rank
  WHERE R = 1
),
count_class AS (
  SELECT champion_name, champion_tier, COUNT(class_id) AS count_class
  FROM high_tier_champ NATURAL JOIN ChampionClasses
  JOIN Classes USING (class_id)
  GROUP BY champion_id
),
class_rank AS (
  SELECT champion_name, champion_tier, count_class, RANK() OVER (
    ORDER BY count_class DESC ) AS RR
  FROM count_class
)
SELECT champion_name, champion_tier, count_class
FROM class_rank
WHERE RR = 1