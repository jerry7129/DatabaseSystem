const db = require('../config/db');

const getUsersPage = async (req, res, next) => {
    const { searchBy, query } = req.query;

    try {
        /*
            TODO: 검색어에 맞춰 유저 목록을 출력하는 페이지를 렌더링하는 코드를 작성하세요.
        */
        let sql = `SELECT user_id, is_admin, penalty FROM Users`;
        let params = [];

        if (query && searchBy === 'username'){
            sql += ` WHERE user_id LIKE ?`;
            params.push(`%${query}%`);
        }
        else if (query && searchBy === 'role'){
            sql += ` WHERE is_admin = ?`;
            let is_admin = -1;
            if (query === 'admin'){
                is_admin = 1;
            }
            else if (query === 'user'){
                is_admin = 0;
            }
            params.push(is_admin);
        }
        sql += ` ORDER BY user_id ASC`;

        const [Users] = await db.query(sql, params);
        const formatUsers = Users.map(user => ({
            id: user.user_id,
            role: user.is_admin ? 'admin' : 'user',
            penalty: user.penalty
        }))

        res.render('pages/users', {
            title: 'User Management',
            users: formatUsers,
            searchBy,
            query
        });
    } catch (err) {
        next(err);
    }
};

const postPayPenalty = async (req, res, next) => {
    const user_id = req.params.id;
    try {
        await db.query(`UPDATE Users SET penalty = 0 WHERE user_id = ?`, [user_id]);
        res.redirect('/borrowings');
    } catch (err) {
        next(err);
    }
};

module.exports = {
    getUsersPage,
    postPayPenalty
};