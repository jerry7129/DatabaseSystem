SELECT avg(champion_tier)
FROM (Champions NATURAL JOIN ChampionOrigins)
JOIN Origins USING (origin_id)
WHERE Origin_name = 'Star Guardian';