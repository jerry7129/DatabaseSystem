SELECT origin_name
FROM Champions NATURAL JOIN ChampionOrigins
JOIN Origins USING (origin_id)
GROUP BY origin_id
HAVING MAX(champion_tier) = 1;