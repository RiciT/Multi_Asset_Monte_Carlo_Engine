using YFinance, Statistics, CSV, DataFrames, Dates

# defining the basket
# later need to find some other more fun ones but for now stick to FAANG

tickers = ["AAPL", "MSFT", "NVDA", "GOOGL", "AMZN", "META"];

endDate = today();
startDate = endDate - Year(1);

println("Fetching a year of data for: ", tickers);

# use the 'close' price to calculate daily log-returns since we are going day by day in our simulation
pricesDF = DataFrame();

for t in tickers
  data = get_prices(t, startdate = startDate);
  pricesDF[!, t] = data["adjclose"];
end
dropmissing!(pricesDF);

priceMatrix = Matrix{Float64}(pricesDF);
returns = diff(log.(priceMatrix, dims=1);

# simple parameter estimation for now
# estimating annualized volatility (standard deviation * \sqrt{252})
volatilities = std(returns, dims=1) .* sqrt(252)
corrMatrix = cor(returns)
spots = Vector(pricesDF[end, :])

CSV.write("market_vols.csv", DataFrame(vols = vec(vols)))
CSV.write("market_spots.csv", DataFrame(spots = spots));
CSV.write("market_correlation.csv", Tables.table(corrMatrix), header=false);

println("succesful data acquisition")
