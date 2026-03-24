SELECT champion_name
FROM Champions NATURAL JOIN ChampionOrigins
JOIN Origins USING (origin_id)
WHERE origin_name IN (
  SELECT origin_name
  FROM Champions NATURAL JOIN ChampionOrigins
  JOIN Origins USING (origin_id)
  WHERE champion_name = 'Aatrox'
) AND champion_name != 'Aatrox'
ORDER BY champion_name ASC;