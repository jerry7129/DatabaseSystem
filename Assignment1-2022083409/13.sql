SELECT origin_name
FROM (Champions NATURAL JOIN ChampionOrigins)
JOIN Origins USING (origin_id)
WHERE champion_id BETWEEN 10 AND 19
ORDER BY champion_id ASC;