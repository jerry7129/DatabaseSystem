SELECT champion_name
FROM Champions
WHERE champion_tier = 3 OR champion_tier = 5
ORDER BY champion_name ASC;