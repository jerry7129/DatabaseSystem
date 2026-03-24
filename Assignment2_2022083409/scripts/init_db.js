const db = require('../config/db');

const initDB = async () => {
    try {
        // Drop existing tables in the correct order
        console.log('Deleting existing tables...');
        // TODO: 기존 테이블 제거하는 코드를 작성하세요.
        await db.query('DROP TABLE IF EXISTS Book_Categories');
        await db.query('DROP TABLE IF EXISTS Book_Authors');

        await db.query('DROP TABLE IF EXISTS Checkout');
        await db.query('DROP TABLE IF EXISTS Books');

        await db.query('DROP TABLE IF EXISTS Users');
        await db.query('DROP TABLE IF EXISTS Book_Info');
        await db.query('DROP TABLE IF EXISTS Categories');
        await db.query('DROP TABLE IF EXISTS Authors');
        // Create tables
        console.log('Creating new tables...');
        // TODO: 설계한 스키마에 맞춰 새로운 테이블을 생성하는 코드를 작성하세요.
        await db.query(`
            CREATE TABLE Users (
                user_id VARCHAR(255) PRIMARY KEY,
                user_pwd VARCHAR(255),
                is_admin BOOLEAN DEFAULT FALSE,
                penalty INT DEFAULT 0
            );
        `);
        await db.query(`
            CREATE TABLE Book_Info (
                info_id INT PRIMARY KEY AUTO_INCREMENT,
                book_title VARCHAR(255) NOT NULL UNIQUE
            );    
        `)
                await db.query(`
            CREATE TABLE Categories (
                category_id INT PRIMARY KEY AUTO_INCREMENT,
                category_name VARCHAR(255) NOT NULL UNIQUE
            );    
        `)
        await db.query(`
            CREATE TABLE Authors (
                author_id INT PRIMARY KEY AUTO_INCREMENT,
                author_name VARCHAR(255) NOT NULL UNIQUE
            );    
        `)

        await db.query(`
            CREATE TABLE Books (
                book_id INT PRIMARY KEY AUTO_INCREMENT,
                info_id INT NOT NULL,
                status ENUM('Available', 'Checkout', 'Lost') NOT NULL DEFAULT 'Available',
                FOREIGN KEY (info_id) REFERENCES Book_Info(info_id)
            );    
        `)
        await db.query(`
            CREATE TABLE Checkout (
                checkout_id INT PRIMARY KEY AUTO_INCREMENT,
                user_id VARCHAR(255) NOT NULL,
                book_id INT NOT NULL,
                checkout_date DATE NOT NULL,
                due_date DATE NOT NULL,
                return_date DATE,
                late_fee INT DEFAULT 0,
                FOREIGN KEY (user_id) REFERENCES Users(user_id),
                FOREIGN KEY (book_id) REFERENCES Books(book_id)
            );    
        `)

        await db.query(`
            CREATE TABLE Book_Categories (
                info_id INT NOT NULL,
                category_id INT NOT NULL,
                PRIMARY KEY (info_id, category_id),
                FOREIGN KEY (info_id) REFERENCES Book_Info(info_id),
                FOREIGN KEY (category_id) REFERENCES Categories(category_id)
            );
        `)
        await db.query(`
            CREATE TABLE Book_Authors (
                info_id INT NOT NULL,
                author_id INT NOT NULL,
                PRIMARY KEY (info_id, author_id),
                FOREIGN KEY (info_id) REFERENCES Book_Info(info_id),
                FOREIGN KEY (author_id) REFERENCES Authors(author_id)
            );    
        `)
        console.log('Database initialization completed successfully.');
    } catch (err) {
        console.error('Database initialization failed:', err);
    } finally {
        db.pool.end();
    }
};

initDB();