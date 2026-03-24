SELECT champion_name
FROM (Champions NATURAL JOIN ChampionOrigins)
JOIN Origins using (origin_id)
WHERE origin_name = 'Wraith' AND champion_tier = 5
ORDER BY champion_name ASC;