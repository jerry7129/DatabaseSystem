SELECT class_name
FROM Classes
WHERE class_name NOT LIKE 'S%'
ORDER BY class_name ASC;