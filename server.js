const http = require('http')

const server = http.createServer((req, res) => {
	let body = ''

	req.on('data', chunk => {
		body += chunk
	})

	req.on('end', () => {
		console.log()
		console.log(new Date().toISOString())
		console.log(`${req.method} ${req.url}`)
		console.log(body || '<empty body>')

		res.writeHead(200, {
			'Content-Type': 'application/json',
		})

		res.end(
			JSON.stringify({
				ok: true,
				receivedAt: new Date().toISOString(),
			}),
		)
	})
})

server.listen(3000, '0.0.0.0', () => {
	console.log('Local API started')
	console.log('http://192.168.1.100:3000')
})
