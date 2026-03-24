const db = require('../config/db');

const getCategoriesPage = async (req, res, next) => {
    try {
        /*
            TODO: 모든 카테고리를 출력하는 페이지를 렌더링하는 코드를 작성하세요.
        */
        const [Categories] = await db.query(`
            SELECT category_id AS id, category_name AS name
            FROM Categories
            ORDER BY category_id ASC    
        `);
        res.render('pages/categories', {
            title: 'Category Management',
            categories: Categories // 카테고리 리스트가 전달되어야 합니다.
        });
    } catch (err) {
        next(err);
    }
};

const postDeleteCategory = async (req, res, next) => {
    const category_id = Number(req.params.id);
    let connect;
    try {
        connect = await db.pool.getConnection();
        await connect.beginTransaction();
        /*
            TODO: 카테고리를 제거하는 코드를 작성하세요.
            만약 해당 카테고리에 포함된 책이 있다면 책에서 해당 카테고리만 지우고 나머지 카테고리는 유지하면 됩니다.
        */
        const [category] = await connect.query(`SELECT category_id FROM Categories WHERE category_id = ?`, [category_id]);
        if (category.length === 0){
            const err = new Error("해당 카테고리를 찾을 수 없습니다.");
            err.status(404);
            await connect.rollback();
            await connect.release();
        }
        await connect.query(`DELETE FROM Book_Categories WHERE category_id = ?`, [category_id]);
        await connect.query(`DELETE FROM Categories WHERE category_id = ?`, [category_id]);
        await connect.commit();
        await connect.release();
        res.redirect('/categories');
    } catch (err) {
        if (connect){
            await connect.rollback();
            await connect.release();
        }
        next(err);
    }
};

module.exports = {
    getCategoriesPage,
    postDeleteCategory
};