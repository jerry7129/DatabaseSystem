SELECT COUNT(champion_id)
FROM Champions
GROUP BY champion_tier
ORDER BY champion_tier ASC;