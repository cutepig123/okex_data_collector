import requests
url = 'https://www.okex.com/api/general/v3/time'
response = requests.get(url)
print(response.json())