WITH spec_champions AS (
  SELECT champion_id
  FROM Champions NATURAL JOIN ChampionOrigins
  JOIN Origins USING (origin_id)
  WHERE origin_name = 'Star Guardian'
),
class_count AS (
  SELECT class_name, COUNT(champion_id) AS C
  FROM spec_champions NATURAL JOIN ChampionClasses
  JOIN Classes USING (class_id)
  GROUP BY class_id
),
class_freq AS (
  SELECT class_name, RANK() OVER (ORDER BY C DESC) AS R
  FROM class_count
)
SELECT class_name
FROM class_freq
WHERE R = 1;