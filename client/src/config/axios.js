const { default: axios } = require("axios")

const headers = localStorage.getItem('token') ? { 'Authorization': `Bearer ${localStorage.getItem('token')}` } : {};

const Axios = axios.create({
  withCredentials: true,
  baseURL: 'http://shavds.shaw.wang:7000',
  headers: headers
})

export default Axios;