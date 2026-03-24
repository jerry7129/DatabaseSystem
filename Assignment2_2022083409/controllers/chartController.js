const db = require('../config/db');

const getChartsPage = async (req, res, next) => {
    let selectedCategoryId = req.query.categoryId ? Number(req.query.categoryId) : null;
    try {
        /*
            TODO: 차트 페이지를 렌더링하는 코드를 작성하세요.
        */
        const [Categories] = await db.query(`
            SELECT category_id AS id, category_name AS name
            FROM Categories
            ORDER BY category_name ASC    
        `);

        if (!selectedCategoryId && Categories.length > 0){
            selectedCategoryId = Categories[0].id;
        }

        const [PopularBooks] = await db.query(`
            WITH checkout_count AS (
                SELECT book_title AS title, GROUP_CONCAT(DISTINCT author_name) AS author, 
                GROUP_CONCAT(DISTINCT category_name) AS categories, COUNT(checkout_id) AS borrow_count
                FROM Books
                JOIN Checkout CH USING (book_id)
                JOIN Book_Info BI USING (info_id)
                LEFT JOIN Book_Categories BC USING (info_id)
                LEFT JOIN Book_Authors BA USING (info_id)
                LEFT JOIN Categories C USING (category_id)
                LEFT JOIN Authors A USING (author_id)
                WHERE CH.checkout_date >= DATE_SUB(CURDATE(), INTERVAL 3 MONTH)
                GROUP BY BI.info_id
            ),
            book_rank AS (
                SELECT title, author, categories, borrow_count, RANK() OVER(ORDER BY borrow_count DESC) AS R
                FROM checkout_count
            )
            SELECT title, author, categories, borrow_count
            FROM book_rank
            ORDER BY R ASC
        `);

        const [popularBooksByCategory] = await db.query(`
            WITH checkout_count AS (
                SELECT info_id, book_title AS title, COUNT(checkout_id) AS borrow_count
                FROM Books
                JOIN Checkout USING (book_id)
                JOIN Book_Info USING (info_id)
                WHERE checkout_date >= DATE_SUB(CURDATE(), INTERVAL 3 MONTH)
                GROUP BY info_id
            ),
            book_concat AS (
                SELECT title, GROUP_CONCAT(DISTINCT author_name) AS author, 
                GROUP_CONCAT(DISTINCT category_name) AS categories, borrow_count, info_id
                FROM checkout_count
                LEFT JOIN Book_Authors USING (info_id)
                LEFT JOIN Authors USING (author_id)
                JOIN Book_Categories USING (info_id)
                JOIN Categories USING (category_id)
                GROUP BY info_id
            ),
            category_filter AS (
                SELECT title, author, categories, borrow_count
                FROM book_concat
                JOIN Book_Categories USING (info_id)
                WHERE category_id = ?
            ),
            book_rank AS (
                SELECT title, author, categories, borrow_count, RANK() OVER(ORDER BY borrow_count DESC) AS R
                FROM category_filter
            )
            SELECT title, author, categories, borrow_count
            FROM book_rank
            ORDER BY R ASC
        `, [selectedCategoryId]);

        const [UserRank] = await db.query(`
            WITH checkout_count AS (
                SELECT user_id AS id, COUNT(checkout_id) AS borrow_count
                FROM Users
                JOIN Checkout CH USING (user_id)
                GROUP BY user_id
            ),
            user_rank AS (
                SELECT id, borrow_count, RANK() OVER(ORDER BY borrow_count DESC) AS ranking
                FROM checkout_count
            )
            SELECT ranking, id, borrow_count
            FROM user_rank
            WHERE ranking <= 5
            ORDER BY ranking ASC
        `);

        const [DeadBooks] = await db.query(`
            SELECT book_title AS title, GROUP_CONCAT(DISTINCT author_name) AS author, 
            GROUP_CONCAT(DISTINCT category_name) AS categories, DATE_FORMAT( MAX(checkout_date), '%Y.%m.%d') AS borrow_date
            FROM Book_Info BI
            JOIN Books USING (info_id)
            LEFT JOIN Checkout USING (book_id)
            LEFT JOIN Book_Categories USING (info_id)
            LEFT JOIN Book_Authors USING (info_id)
            LEFT JOIN Categories USING (category_id)
            LEFT JOIN Authors USING (author_id)
            GROUP BY BI.info_id
            ORDER BY borrow_date ASC
            LIMIT 5
        `);

        res.render('pages/charts', {
            title: 'Charts',
            popularBooks: PopularBooks,
            popularBooksByCategory: popularBooksByCategory,
            categories: Categories,
            userRank: UserRank,
            deadBooks: DeadBooks,
            selectedCategoryId
        });
    } catch (err) {
        next(err);
    }
};

module.exports = {
    getChartsPage
};