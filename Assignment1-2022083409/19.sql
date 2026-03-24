WITH count_and_avg AS (
  SELECT origin_name, COUNT(champion_id) AS C, AVG(champion_tier) AS A
  FROM Champions NATURAL JOIN ChampionOrigins
  JOIN Origins USING (origin_id)
  GROUP BY origin_id
)
SELECT origin_name
FROM count_and_avg
WHERE C >= 4 AND A >= 2.5
ORDER BY origin_name;