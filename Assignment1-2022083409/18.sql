WITH count_list AS(
  SELECT class_name, COUNT(champion_id) AS champion_count
  FROM Champions NATURAL JOIN Championclasses
  JOIN Classes Using (class_id)
  GROUP BY class_id
),
class_rank AS(
  SELECT class_name, RANK() OVER (ORDER BY champion_count ASC) AS champion_count_rank
  FROM count_list
)
SELECT class_name
FROM class_rank
WHERE champion_count_rank = 1
ORDER BY class_name;
