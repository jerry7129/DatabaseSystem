SELECT COUNT(champion_id)
FROM (Champions NATURAL JOIN ChampionOrigins)
JOIN Origins USING (origin_id)
WHERE origin_name != 'Battle Academia';