# Load required libraries
library(dplyr)
library(lubridate)

# Read the dataset
data <- read.csv("AAPL.csv")

# Convert the Date column to Date format
data$Date <- as.Date(data$Date, format = "%Y-%m-%d")

# Check for missing values and handle them (for example, by removing rows with missing values)
data_cleaned <- data %>%
  drop_na()

# Check for duplicates and remove them if needed
data_cleaned <- data_cleaned %>%
  distinct()

# Convert 'Volume' to numeric if needed (ensure it's in correct format)
data_cleaned$Volume <- as.numeric(data_cleaned$Volume)

# Preview the cleaned data
head(data_cleaned)
