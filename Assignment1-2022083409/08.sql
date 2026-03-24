SELECT champion_name, champion_tier
FROM Champions NATURAL JOIN ChampionOrigins NATURAL JOIN Origins
WHERE origin_name = 'Mighty Mech'
ORDER BY champion_name ASC;