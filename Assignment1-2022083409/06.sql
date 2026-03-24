SELECT champion_name
FROM Champions
WHERE champion_name LIKE '%a' OR
      champion_name LIKE '%e' OR
      champion_name LIKE '%o'
ORDER BY champion_name ASC;