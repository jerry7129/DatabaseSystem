const db = require('../config/db'); 

const getBorrowingsPage = async (req, res, next) => {
    const user_id = req.session.userId;

    if (!user_id) {
        return res.redirect('/login');
    }

    try {
        /*
            TODO: 유저의 대여 기록을 모두 출력하는 페이지를 렌더링하는 코드를 작성하세요.
        */
        const [Borrowings] = await db.query(`
            SELECT 
                checkout_id AS id, 
                book_id, 
                book_title, 
                GROUP_CONCAT(DISTINCT author_name) AS book_author, 
                DATE_FORMAT(checkout_date, '%Y.%m.%d') AS borrow_date, 
                DATE_FORMAT(return_date, '%Y.%m.%d') AS return_date, 
                status
            FROM Checkout
            JOIN Books USING (book_id)
            JOIN Book_Info USING (info_id)
            LEFT JOIN Book_Categories USING (info_id)
            LEFT JOIN Categories USING (category_id)
            LEFT JOIN Book_Authors USING (info_id)
            LEFT JOIN Authors USING (author_id)
            WHERE user_id = ?
            GROUP BY checkout_id
            ORDER BY checkout_id ASC
        `, [user_id])

        const [Penalty] = await db.query(`
            SELECT user_id AS id, penalty
            FROM Users
            WHERE user_id = ?
        `, [user_id]);

        res.render('pages/borrowings', {
            title: 'My Borrowing History',
            borrowings: Borrowings, // 대여 기록 리스트가 전달되어야 합니다.
            penalty: Penalty[0]
        });
    } catch (err) {
        next(err);
    }
};

module.exports = {
    getBorrowingsPage
};