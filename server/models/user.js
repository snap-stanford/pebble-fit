var mongoose = require('mongoose');

var userSchema = new mongoose.Schema({
  watch             : { type: String, required: true},
  group             : { type: String, required: true},
  name              : { type: String },
  email             : { type: String },
  age               : { type: Number },
  gender            : { type: String },
  height            : { type: String },
  heightU           : { type: String },
  weight            : { type: Number },
  weightU           : { type: String },
  race              : { type: String },
  school            : { type: String },
  occupation        : { type: String },
  deskwork          : { type: String },
  income            : { type: String },
  country           : { type: String },
  zipcode           : { type: String },
  createdAt         : {type: Date, default: Date.now},
  configUpdatedAt   : {type: Date, default: Date.now},
});

module.exports = mongoose.model('User', userSchema);
