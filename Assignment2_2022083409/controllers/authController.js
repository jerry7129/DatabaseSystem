const db = require('../config/db');
const adminCode = '2022083409' // TODO: admin code(본인 학번)를 추가하세요.
const bcrypt = require('bcrypt');

const getLoginPage = (req, res) => {
    res.render('pages/login', { title: 'Login' });
};

const getRegisterPage = (req, res) => {
    res.render('pages/register', { title: 'Register' });
};

const logoutAndGetHomePage = (req, res, next) => {
    req.session.destroy(err => {
        if (err) {
            return next(err);
        }
        res.redirect('/');
    });
};

const postLogin = async (req, res, next) => {
    const { username, password } = req.body;
    try {
        const [users] = await db.query(`
              SELECT * FROM Users WHERE user_id = ?`, [username]);
        if (users.length === 0){
          const err = Error('아이디 또는 비밀번호가 잘못되었습니다.');
          err.status = 401;
          return next(err);
        }
        
        const is_match = await bcrypt.compare(password, users[0].user_pwd);
        if(!is_match){
          const err = Error('아이디 또는 비밀번호가 잘못되었습니다.');
          err.status = 401;
          return next(err);
        }

        const user = users[0];
        const role = user.is_admin ? 'admin' : 'user';

        req.session.userId = user.user_id;
        req.session.role = role;
        req.session.penalty = user.penalty;

        res.redirect('/');
    } catch (err) {
        return next(err);
    }
};

const postRegister = async (req, res, next) => {
    const { username, password, admin_code, role  } = req.body;
    const try_admin = role === 'admin';
    const penalty = 0;
    try {
        const [same_id] = await db.query(`SELECT user_id
                                          FROM Users
                                          WHERE user_id = ?`, [username]);
        if (same_id.length > 0){
          const err = new Error('이미 존재하는 아이디입니다.');
          err.status = 409; // Conflict
          return next(err);
        }
        if(try_admin && admin_code !== adminCode){
          const err = new Error('관리자 번호가 틀렸습니다.')
          err.status = 401; // Unauthorized
          return next(err);
        }
        const is_admin = try_admin && (admin_code === adminCode);
        const hashed_pwd = await bcrypt.hash(password, 10);

        const sql = `INSERT INTO Users (user_id, user_pwd, is_admin, penalty) VALUES (?, ?, ?, ?)`
        await db.query(sql, [username, hashed_pwd, is_admin, penalty]);

        res.redirect('/login');
    } catch (err) {
        return next(err);
    }
};

module.exports = {
    getLoginPage,
    getRegisterPage,
    logoutAndGetHomePage,
    postLogin,
    postRegister,
};