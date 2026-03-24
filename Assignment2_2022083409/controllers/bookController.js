const db = require('../config/db');

const getBooksPage = async (req, res, next) => {
    const { query: searchQuery, searchBy, sortBy, sortOrder } = req.query;
    const sortedByWithDefault = sortBy || 'title';
    const sortOrderWithDefault = sortOrder || 'asc';

    try {
        /*
            TODO: 검색어, 정렬 기준에 맞춰 책 목록을 출력하는 페이지를 렌더링하는 코드를 작성하세요.
        */
        let sql = `
            SELECT
                BI.info_id AS id,
                BI.book_title AS title, 
                GROUP_CONCAT(DISTINCT A.author_name) AS author,
                GROUP_CONCAT(DISTINCT C.category_name) AS categories,
                COUNT(DISTINCT B.book_id) AS total_quantity,
                COUNT(DISTINCT CASE WHEN B.status = 'Available' THEN B.book_id ELSE NULL END) AS available_quantity
            FROM Book_Info BI
            JOIN Books B USING (info_id)
            LEFT JOIN Book_Categories BC USING (info_id)
            LEFT JOIN Categories C USING (category_id)
            LEFT JOIN Book_Authors BA USING (info_id)
            LEFT JOIN Authors A USING (author_id)
        `

        let params = [];

        if (searchQuery && searchBy){ // 검색어 조건이 있는 경우
            if (searchBy === 'title'){
                sql += ` WHERE book_title LIKE ?`;
                params += `%${searchQuery}%`;
            } else if (searchBy === 'author'){
                sql += `
                    WHERE BI.info_id IN (
                    SELECT info_id
                    FROM Book_Authors
                    JOIN Authors USING (author_id)
                    WHERE author_name LIKE ?
                )`;
                params += `%${searchQuery}%`;
            } else if (searchBy === 'category'){
                sql += `
                    WHERE BI.info_id IN (
                    SELECT info_id
                    FROM Book_Categories
                    JOIN Categories USING (category_id)
                    WHERE category_name LIKE ?
                )`;
                params += `%${searchQuery}%`;
            }
        }
        
        sql += `
            GROUP BY BI.info_id
            ORDER BY ${sortedByWithDefault} ${sortOrderWithDefault.toUpperCase()};
        `;
        const [books] = await db.query(sql, params);

        res.render('pages/books', {
            title: 'All Books',
            books: books, //dummyBooks, // 정렬된 검색 결과 리스트가 전달되어야 합니다.
            sortBy: sortedByWithDefault,
            sortOrder: sortOrderWithDefault,
            query: searchQuery,
            searchBy: searchBy
        });
    } catch (err) {
        next(err);
    }
};


const getAddBookPage = async (req, res, next) => {
    try {
        /*
            TODO: 책을 추가하는 페이지를 렌더링 하는 코드를 작성하세요.
            책 추가 시 작가와 카테고리를 선택해야하므로 현재 카테고리 목록과 작가 목록을 불러와야 합니다.
        */
        const [Categories] = await db.query(`
            SELECT category_id AS id, category_name AS name 
            FROM Categories 
            ORDER BY category_name ASC
        `);
        const [Authors] = await db.query(`
            SELECT author_id AS id, author_name AS name 
            FROM Authors 
            ORDER BY author_name ASC
        `);
        res.render('pages/add-book', {
            title: 'Add New Book',
            categories: Categories, // 카테고리 리스트가 전달되어야 합니다.
            authors: Authors, // 저자 리스트가 전달되어야 합니다.
        });
    } catch (err) {
        next(err);
    }
};


const postAddBook = async (req, res, next) => {
    const { title, authors, quantity: quantity_str, categories } = req.body;
    const quantity = parseInt(quantity_str, 10)

    // authors, categories는 multiple 속성 때문에 보통 배열로 오지만, 
    // 입력이 하나인 경우 단일 문자열로 오는 경우를 대비해 배열로 통일.
    const author_array = Array.isArray(authors) ? authors : [authors].filter(a => a);
    const category_array = Array.isArray(categories) ? categories : [categories].filter(a => a);

    /*
        TODO: 책을 DB에 추가하는 작업을 수행하는 코드를 작성하세요.
        기존에 없는 카테고리와 저자 또한 추가해줘야 합니다.
    */
    let connect;

    try{
        connect = await db.pool.getConnection();
        await connect.beginTransaction();

        //Book_Info에 추가
        const [Book_Info_Result] = await connect.query(`INSERT INTO Book_Info (book_title) VALUES (?)`, [title]);
        const info_id = Book_Info_Result.insertId;

        for (const author of author_array){ //Authors에 추가 또는 기존꺼 재사용
            let author_id;
            if (author.startsWith('id:')){
                author_id = author.substring(3);
            } else{
                const [same_author] = await connect.query(`SELECT author_id 
                                                           FROM Authors 
                                                           WHERE author_name = ?`, [author]);
                if (same_author.length > 0){
                    author_id = same_author[0].author_id;
                } else{
                    const [Author_Result] = await connect.query(`INSERT INTO Authors (author_name) VALUES (?)`, [author]);
                    author_id = Author_Result.insertId;
                }
            }
            //Book_Info와 Authors를 연결 테이블을 통해 연결
            await connect.query(`INSERT INTO Book_Authors (info_id, author_id) VALUES (?, ?)`, [info_id, author_id]);
        }

        for (const category of category_array){ //Categories에 추가 또는 기존꺼 재사용
            let category_id;
            if (category.startsWith('id:')){
                category_id = category.substring(3);
            } else{
                const [same_category] = await connect.query(`SELECT category_id
                                                             FROM Categories
                                                             WHERE category_name = ?`, [category]);
                if (same_category.length > 0){
                    category_id = same_category[0].category_id;
                } else{
                    const [Category_Result] = await connect.query(`INSERT INTO Categories (category_name) VALUES (?)`, [category]);
                    category_id = Category_Result.insertId;
                }
            }
            //Book_Info와 Categories를 연결 테이블을 통해 연결
            await connect.query(`INSERT INTO Book_Categories (info_id, category_id) VALUES (?, ?)`, [info_id, category_id]);
        }

        for (let i = 0; i < quantity; i++){
            await connect.query(`INSERT INTO Books (info_id, status) VALUES (?, "Available")`,[info_id]);
        }
        await connect.commit();
        await connect.release();
        res.redirect('/books');

    } catch (err){
        if (connect){
            await connect.rollback();
            await connect.release();
        }
        next(err);
    }
};


const postDeleteBookInstance = async (req, res, next) => {
    const book_id = Number(req.params.id);

    let connect;
    try {
        connect = await db.pool.getConnection();
        await connect.beginTransaction();
        /*
            TODO: 책 한 권을 제거하는 작업을 수행하는 코드를 작성하세요.
            동일한 책을 모두 제거하면 해당 책에 대한 정보도 지워지도록 구현해주세요.
        */
        const [instance] = await connect.query(`SELECT info_id FROM Books WHERE book_id = ?`, [book_id]);
        if(instance.length === 0){
            const err = new Error('해당 책을 찾을 수 없습니다.');
            err.status(404);
            await connect.rollback();
            await connect.release();
            return next(err);
        }
        const info_id = instance[0].info_id;
        await connect.query(`DELETE FROM Books WHERE book_id = ?`, [book_id]);
        const [count] = await connect.query(`SELECT COUNT(book_id) AS count FROM Books Where info_id = ?`, [info_id]);
        if (count[0].count === 0){
            await connect.query(`DELETE FROM Book_Categories WHERE info_id = ?`, [info_id]);
            await connect.query(`DELETE FROM Book_Authors WHERE info_id = ?`, [info_id]);
            await connect.query(`DELETE FROM Book_Info WHERE info_id = ?`, [info_id]);
        }

        await connect.commit();
        await connect.release();
        res.redirect('/books');
    } catch (err) {
        if(connect){
            await connect.rollback();
            await connect.release();
        }
        next(err);
    }
};


const postBorrowBook = async (req, res, next) => {
    const book_id = Number(req.params.id);
    const user_id = req.session.userId;

    if (!user_id) {
        return res.redirect('/login');
    }

    let connect;
    try {
        connect = await db.pool.getConnection();
        await connect.beginTransaction();
        /*
            TODO: 특정 책을 대여하는 작업을 수행하는 코드를 작성하세요.
            명세에 있는 조건들을 어기는 작업일 경우에는 다음과 같이 에러페이지로 유도하면 됩니다.

            ```
                const err = new Error('You have reached the maximum borrowing limit (3 books).');
                err.status = 400;
                return next(err);
            ```
        */
        const [info] = await connect.query('SELECT info_id FROM Books WHERE book_id = ?', [book_id]);
        const info_id = info[0].info_id
        const [user] = await connect.query(`
            SELECT COUNT(checkout_id) AS count,
                SUM(CASE WHEN CURDATE() > due_date THEN 1 ELSE 0 END) AS is_late,
                SUM(CASE WHEN info_id = ? THEN 1 ELSE 0 END) AS is_same_book
            FROM Users 
            LEFT JOIN Checkout USING (user_id)
            LEFT JOIN Books USING (book_id)
            WHERE user_id = ? AND return_date IS NULL
        `, [info_id, user_id]);
        const [penalty_fee] = await connect.query(`SELECT penalty FROM Users WHERE user_id = ?`, [user_id]);
        if (user[0].is_late > 0){
            const err = new Error("현재 연체된 책이 있어 대출 할 수 없습니다.");
            err.status = 403;
            await connect.rollback();
            await connect.release();
            return next(err);
        }
        if (penalty_fee[0].penalty > 0){
            const err = new Error("현재 미납된 요금이 있어 대출 할 수 없습니다.");
            err.status = 403;
            await connect.rollback();
            await connect.release();
            return next(err);
        }
        if (user[0].is_same_book > 0){
            const err = new Error("같은 책을 두 권 이상 대출 할 수 없습니다.");
            err.status = 400;
            await connect.rollback();
            await connect.release();
            return next(err);
        }
        if (user[0].count >= 3){
            const err = new Error("최대 대출 가능 권수(3권)를 초과했습니다.");
            err.status = 400;
            await connect.rollback();
            await connect.release();
            return next(err);
        }
        
        const [book] = await connect.query(`SELECT status FROM Books WHERE book_id = ?`, [book_id]);
        if (book.length === 0 || book[0].status !== 'Available'){
            const err = new Error("해당 책을 대출할 수 없습니다.");
            err.status = 400;
            await connect.rollback();
            await connect.release();
            return next(err);
        }

        await connect.query(`
            INSERT INTO Checkout (user_id, book_id, checkout_date, due_date) 
            VALUES (?, ?, CURDATE(), DATE_ADD(CURDATE(), INTERVAL 6 DAY))
        `, [user_id, book_id]);

        await connect.query(`UPDATE Books SET status = "Checkout" WHERE book_id = ?`, [book_id]);

        await connect.commit();
        await connect.release();
        res.redirect('/books');
    } catch (err) {
        if (connect){
            await connect.rollback();
            await connect.release();
        }
        next(err);
    }
};


const postReturnBook = async (req, res, next) => {
    const checkout_id = Number(req.params.id);
    const user_id = req.session.userId;

    if (!user_id) {
        return res.redirect('/login');
    }

    let connect;
    try {
        connect = await db.pool.getConnection();
        await connect.beginTransaction();
        /*
            TODO: 자신이 책을 빌린 기록을 반납 처리하는 코드를 작성해주세요.
            다른 사람이 빌린 책은 반납할 수 없어야 합니다.
        */
        const [checkout] = await connect.query(`
            SELECT * 
            FROM Checkout 
            WHERE checkout_id = ? AND return_date IS NULL
        `, [checkout_id]);

        if (checkout.length === 0){
            const err = new Error("대출 기록을 찾을 수 없습니다.");
            err.status(404);
            await connect.rollback();
            await connect.release();
            return next(err);
        }

        if (checkout[0].user_id !== user_id){
            const err = new Error("Access Denied: 본인이 빌린 책만 반납할 수 있습니다.");
            err.status(403);
            await connect.rollback();
            await connect.release();
            return next(err);
        }

        const {book_id, due_date} = checkout[0];
        const return_date = new Date();
        const dueDate = new Date(due_date);
        let late_fee = 0;

        if (dueDate < return_date){
            const diff_time = return_date.getTime() - dueDate.getTime();
            const diff_days = Math.floor(diff_time / (1000 * 60 * 60 * 24)); // 연체 일수 계산

            late_fee = diff_days * 100; // 연체료는 연체 일당 100원
            await connect.query(`UPDATE Users SET penalty = penalty + ? WHERE user_id = ?`, [late_fee, user_id]);
        }

        // Checkout과 Books 업데이트
        connect.query(`UPDATE Checkout SET return_date = CURDATE(), late_fee = ? WHERE checkout_id = ?`, [late_fee, checkout_id]);
        connect.query(`UPDATE Books SET status = 'Available' WHERE book_id = ?`, [book_id]);

        await connect.commit();
        await connect.release();
        res.redirect('/borrowings');
    } catch (err) {
        if (connect){
            await connect.rollback();
            await connect.release();
        }
        next(err);
    }
};


const getBookInstances = async (req, res, next) => {
    const info_id = Number(req.params.id);

    try {
        /*
            TODO: 특정 동일한 책의 개별 리스트를 불러오는 코드를 작성해주세요.
        */
        const [Instances] = await db.query(`
            SELECT 
                book_id AS id, checkout_id AS borrowing_id, user_id AS borrowed_by, 
                DATE_FORMAT(checkout_date, '%Y.%m.%d') AS borrow_date, status
            FROM Books B
            LEFT JOIN Checkout USING (book_id)
            WHERE info_id = ? 
                AND (
                    checkout_id IS NULL
                    OR checkout_id = (
                        SELECT MAX(checkout_id)
                        FROM Checkout
                        WHERE book_id = B.book_id
                    )
                )
            ORDER BY book_id ASC
        `, [info_id]);
        res.json(Instances);
    } catch (err) {
        next(err);
    }
};

module.exports = {
    getBooksPage,
    getAddBookPage,
    postAddBook,
    postDeleteBookInstance,
    postBorrowBook,
    postReturnBook,
    getBookInstances
};