var express = require('express');
var router = express.Router();

/* GET home page. */
router.get('/', function(req, res, next) {
  res.render('index', { title: 'Express' });
});

router.get('/collect', function (req, res, next) {
  console.log(req.query.data)
  return res.json({'message': 'ok'})
})

module.exports = router;
